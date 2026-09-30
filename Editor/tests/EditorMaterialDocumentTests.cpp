#include <base/ZEngine.h>
#include <editor/EditorMaterialService.h>
#include <gtest/gtest.h>
#include <chrono>
#include <fstream>
#include <limits>

using namespace EditorWorker;
using namespace RenderWorker;
namespace fs = std::filesystem;
namespace
{
class MaterialWorkflow : public testing::Test
{
protected:
    fs::path root;
    std::unique_ptr<EditorMaterialService> service;
    void SetUp() override {
        root = fs::temp_directory_path() / ("zengine-workflow-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        ASSERT_TRUE(fs::create_directory(root));
        fs::create_directory(root / "Assets");
        fs::create_directory(root / "Assets/Materials");
        service = std::make_unique<EditorMaterialService>(root / "Assets");
    }
    void TearDown() override {
        service.reset();
        std::error_code ec;
        if (fs::equivalent(root.parent_path(), fs::temp_directory_path(), ec) && root.filename().string().starts_with("zengine-workflow-test-")) fs::remove_all(root, ec);
    }
    void Write(char const* name, char const* content) { std::ofstream file(root / name, std::ios::binary); file << content; ASSERT_TRUE(file.good()); }
    MaterialDocumentPtr Create() { return service->Create("Materials/Demo.mat"); }
    void Edit(MaterialDocumentPtr const& d, float value = 0.7f) { auto p = d->Parameters(); p.metalness = value; d->Edit(p); }
};
TEST_F(MaterialWorkflow, RejectsEscapeAndInvalidPaths)
{
    for (auto path : {"../bad.mat", "Materials/../../bad.mat", "/outside.mat", "C:/outside.mat", "a:b.mat", "bad?.mat", "", "."})
        EXPECT_THROW(service->Resolve(path), std::runtime_error) << path;
    EXPECT_THROW(service->Create("wrong.drmat"), std::runtime_error);
}
TEST_F(MaterialWorkflow, RejectsEscapingSymlink)
{
    std::error_code ec;
    fs::create_directory_symlink(root, root / "Assets/escape", ec);
    if (ec) GTEST_SKIP() << "Directory symlink creation unavailable: " << ec.message();
    EXPECT_THROW(service->Resolve("escape/outside.mat"), std::runtime_error);
}
TEST_F(MaterialWorkflow, UnicodeDocument)
{
    auto path = MaterialUtf8(fs::path(u8"Materials/金属材质.mat"));
    auto doc = service->Create(path);
    auto p = doc->Parameters(); p.name = MaterialUtf8(fs::path(u8"红色材质")); doc->Edit(p); service->Save(*doc);
    EXPECT_EQ(doc, service->Open(path)); service->Reload(*doc);
    EXPECT_EQ(p.name, doc->Material().Name()); EXPECT_FALSE(doc->Dirty());
}
TEST_F(MaterialWorkflow, CanonicalIdentityAndSingleDocument)
{
    auto d = Create();
    EXPECT_EQ(d, service->Open("./Materials/./Demo.mat"));
#ifdef _WIN32
    EXPECT_EQ(d, service->Open("materials/DEMO.mat"));
#endif
}
TEST_F(MaterialWorkflow, DefaultsAndCollision)
{
    auto d = Create();
    EXPECT_EQ("EditorLitV1", d->Material().Template());
    EXPECT_EQ(0.8f, d->Parameters().albedo[0]); EXPECT_EQ(0.5f, d->Parameters().glossiness);
    EXPECT_FALSE(d->Dirty());
    EXPECT_THROW(Create(), std::runtime_error);
    EXPECT_THROW(service->Create("missing/fail.mat"), std::exception);
    EXPECT_EQ(1u, service->Documents().size());
}
TEST_F(MaterialWorkflow, ContextCreateUsesProjectFolderAndRoot)
{
    auto doc = service->CreateInFolder("Materials", "Context.mat");
    EXPECT_EQ(service->Identity("Materials/Context.mat"), doc->Path());
    EXPECT_TRUE(fs::exists(root / "Assets/Materials/Context.mat"));
    EXPECT_EQ("EditorLitV1", doc->Material().Template());
    EXPECT_FALSE(doc->Dirty());
    EXPECT_EQ(doc, service->Open("Materials/Context.mat"));
    auto root_doc = service->CreateInFolder(".", "Root.mat");
    EXPECT_TRUE(fs::exists(root / "Assets/Root.mat"));
    EXPECT_EQ(service->Identity("Root.mat"), root_doc->Path());
}
TEST_F(MaterialWorkflow, ContextCreateRejectsInvalidNamesAndPreservesExisting)
{
    auto doc = service->CreateInFolder("Materials", "Context.mat");
    auto values = doc->Parameters(); values.metalness = 0.7f; doc->Edit(values); service->Save(*doc);
    EXPECT_THROW(service->CreateInFolder("Materials", "Context.mat"), std::runtime_error);
    for (auto name : {"", "../Escape.mat", "sub/Nested.mat", "sub\\Nested.mat", "C:bad.mat", "Bad?.mat", "Bad.drmat"})
        EXPECT_THROW(service->CreateInFolder("Materials", name), std::runtime_error) << name;
    EXPECT_THROW(service->CreateInFolder("..", "Escape.mat"), std::runtime_error);
    EXPECT_THROW(service->CreateInFolder("missing", "Missing.mat"), std::exception);
    EXPECT_EQ(1u, service->Documents().size());
    EXPECT_EQ(0.7f, LoadRenderMaterialFile(root / "Assets/Materials/Context.mat", false)->Metalness());
}
TEST_F(MaterialWorkflow, EditIsolationDirtyAndRevert)
{
    auto d = Create();
    Edit(d);
    EXPECT_TRUE(d->Dirty());
    EXPECT_EQ(0, LoadRenderMaterialFile(root / "Assets/Materials/Demo.mat", false)->Metalness());
    auto version = d->Version();
    Edit(d);
    EXPECT_EQ(version, d->Version());
    Edit(d, 0); EXPECT_FALSE(d->Dirty());
    Edit(d); d->Revert(); EXPECT_FALSE(d->Dirty()); EXPECT_EQ(0, d->Material().Metalness());
}
TEST_F(MaterialWorkflow, SaveAndFreshReload)
{
    auto d = Create();
    auto file = MaterialUtf8(root / "Assets/Materials/Demo.mat");
    auto cached = SyncLoadRenderMaterial(file);
    Edit(d); service->Save(*d);
    EXPECT_EQ(0, cached->Metalness());
    EXPECT_EQ(0.7f, SyncLoadRenderMaterial(file)->Metalness());
    EXPECT_FALSE(d->Dirty());
    EXPECT_EQ(0.7f, LoadRenderMaterialFile(root / "Assets/Materials/Demo.mat", false)->Metalness());
    Write("Assets/Materials/Demo.mat", "<material><metalness value=\"0.2\"/></material>");
    service->Reload(*d); EXPECT_EQ(0.2f, d->Material().Metalness());
    Edit(d);
    Write("Assets/Materials/Demo.mat", "broken");
    EXPECT_THROW(service->Reload(*d), std::runtime_error);
    EXPECT_TRUE(d->Dirty()); EXPECT_EQ(0.7f, d->Material().Metalness());
}
TEST_F(MaterialWorkflow, FailedSaveStaysDirty)
{
    auto d = Create(); Edit(d);
    fs::rename(root / "Assets/Materials", root / "Assets/Moved");
    EXPECT_THROW(service->Save(*d), std::exception); EXPECT_TRUE(d->Dirty());
    EXPECT_THROW(service->Close(d, MaterialCloseChoice::Save), std::exception);
    EXPECT_EQ(1u, service->Documents().size());
    EXPECT_FALSE(service->ResolveExit(MaterialCloseChoice::Cancel));
    EXPECT_THROW(service->ResolveExit(MaterialCloseChoice::Save), std::exception);
    EXPECT_TRUE(d->Dirty());
}
TEST_F(MaterialWorkflow, SaveAsIsIndependent)
{
    auto d = Create(); Edit(d);
    auto copy = service->SaveAs(*d, "Materials/Copy.mat");
    EXPECT_NE(d, copy); EXPECT_TRUE(d->Dirty()); EXPECT_FALSE(copy->Dirty());
    Edit(copy, 0.3f); EXPECT_EQ(0.7f, d->Material().Metalness());
    EXPECT_EQ(0, LoadRenderMaterialFile(root / "Assets/Materials/Demo.mat", false)->Metalness());
    EXPECT_THROW(service->SaveAs(*d, "Materials/Copy.mat"), std::runtime_error);
    auto m = LoadRenderMaterialFile(root / "Assets/Materials/Demo.mat", false);
    m->Metalness(0.9f);
    EXPECT_THROW(SaveRenderMaterial(m, MaterialUtf8(root / "Assets/Materials/Copy.mat"), false), std::runtime_error);
    EXPECT_EQ(0.7f, LoadRenderMaterialFile(root / "Assets/Materials/Copy.mat", false)->Metalness());
}
TEST_F(MaterialWorkflow, SaveDiscardCancel)
{
    auto d = Create(); Edit(d);
    EXPECT_FALSE(service->Close(d, MaterialCloseChoice::Cancel)); EXPECT_TRUE(d->Dirty());
    EXPECT_EQ(d, service->Open(d->Path()));
    EXPECT_TRUE(service->Close(d, MaterialCloseChoice::Save));
    d = service->Open("Materials/Demo.mat"); EXPECT_EQ(0.7f, d->Material().Metalness());
    Edit(d, 0.4f); EXPECT_TRUE(service->Close(d, MaterialCloseChoice::Discard));
    d = service->Open("Materials/Demo.mat"); EXPECT_EQ(0.7f, d->Material().Metalness());
    Edit(d, 0.9f); EXPECT_TRUE(service->HasDirty());
    EXPECT_FALSE(service->ResolveExit(MaterialCloseChoice::Cancel)); EXPECT_TRUE(service->HasDirty());
    EXPECT_TRUE(service->ResolveExit(MaterialCloseChoice::Discard)); EXPECT_FALSE(service->HasDirty());
    Edit(d, 0.4f); EXPECT_TRUE(service->ResolveExit(MaterialCloseChoice::Save)); EXPECT_FALSE(service->HasDirty());
    EXPECT_EQ(0.4f, LoadRenderMaterialFile(root / "Assets/Materials/Demo.mat", false)->Metalness());
}
TEST_F(MaterialWorkflow, ParameterValidationAndReset)
{
    auto d = Create(); auto original = d->Parameters();
    for (auto value : {-1.f, 1.1f, std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()}) {
        auto p = original; p.metalness = value;
        EXPECT_THROW(d->Edit(p), std::runtime_error);
        EXPECT_FALSE(d->Dirty());
    }
    Edit(d); d->ResetParameters(); EXPECT_FALSE(d->Dirty());
    EXPECT_EQ("Demo", d->Material().Name());
}
TEST_F(MaterialWorkflow, TextureNoopClearAndFailure)
{
    Write("Assets/good.png", "test placeholder (headless loader)");
    Write("Assets/bad.png", "bad");
    auto d = Create();
    service->SetTexture(*d, RenderMaterial::TS_Albedo, "good.png");
    EXPECT_TRUE(d->Dirty()); auto version = d->Version();
    service->SetTexture(*d, RenderMaterial::TS_Albedo, "./good.png"); EXPECT_EQ(version, d->Version());
    EXPECT_THROW(service->SetTexture(*d, RenderMaterial::TS_Albedo, "missing.png"), std::runtime_error);
    EXPECT_THROW(service->SetTexture(*d, RenderMaterial::TS_Albedo, "Materials/Demo.mat"), std::runtime_error);
    EXPECT_EQ("good.png", d->Material().TextureName(RenderMaterial::TS_Albedo));
    service->SetTexture(*d, RenderMaterial::TS_Albedo, ""); EXPECT_FALSE(d->Dirty());
    int calls = 0;
    EditorMaterialService failing(root / "Assets", [&](fs::path const&) -> ShaderResourceViewPtr { ++calls; throw std::runtime_error("decoder failure"); });
    auto fresh = failing.Open("Materials/Demo.mat");
    EXPECT_THROW(failing.SetTexture(*fresh, RenderMaterial::TS_Albedo, "bad.png"), std::runtime_error);
    EXPECT_EQ(1, calls); EXPECT_FALSE(fresh->Dirty());
    EXPECT_TRUE(fresh->Material().TextureName(RenderMaterial::TS_Albedo).empty());
    int loaded = 0;
    EditorMaterialService counting(root / "Assets", [&](fs::path const& path) {
        ++loaded;
        if (path.filename() == "bad.png") throw std::runtime_error("decoder failure");
        return std::make_shared<ShaderResourceView>();
    });
    auto good = counting.Open("Materials/Demo.mat");
    counting.SetTexture(*good, RenderMaterial::TS_Albedo, "good.png");
    auto last_good = good->Material().Texture(RenderMaterial::TS_Albedo);
    counting.SetTexture(*good, RenderMaterial::TS_Albedo, "./good.png");
    EXPECT_EQ(1, loaded);
    EXPECT_THROW(counting.SetTexture(*good, RenderMaterial::TS_Albedo, "missing.png"), std::runtime_error);
    EXPECT_EQ(last_good, good->Material().Texture(RenderMaterial::TS_Albedo));
    EXPECT_THROW(counting.SetTexture(*good, RenderMaterial::TS_Albedo, "bad.png"), std::runtime_error);
    EXPECT_EQ(2, loaded); EXPECT_EQ(last_good, good->Material().Texture(RenderMaterial::TS_Albedo));
    EXPECT_EQ("good.png", good->Material().TextureName(RenderMaterial::TS_Albedo));
}
TEST_F(MaterialWorkflow, TemplateRoundTripAndWhitelist)
{
    auto d = Create();
    auto m = LoadRenderMaterialFile(root / "Assets/Materials/Demo.mat", false);
    EXPECT_EQ("EditorLitV1", m->Template()); EXPECT_EQ("EditorLitV1.shader", m->EffectName());
    EXPECT_EQ("EditorLitV1", m->Clone()->Template());
    m->Template(""); EXPECT_EQ("SimpleAlbedoNormal.shader", m->EffectName());
    EXPECT_THROW(m->Template("../custom.shader"), std::runtime_error);
    Write("Assets/unknown.mat", "<material template=\"Unknown\"/>");
    EXPECT_THROW(service->Open("unknown.mat"), std::runtime_error);
}
}
