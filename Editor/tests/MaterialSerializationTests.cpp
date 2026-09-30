#include <base/ZEngine.h>
#include <render/RenderMaterial.h>
#include <render/detail/MaterialFileIO.h>
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <limits>

using namespace RenderWorker;
namespace fs = std::filesystem;

namespace
{
class MaterialIO : public testing::Test
{
protected:
    fs::path root;
    void SetUp() override
    {
        static std::atomic<unsigned> serial {0};
        root = fs::temp_directory_path() / ("zengine-material-test-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "-" + std::to_string(serial++));
        ASSERT_TRUE(fs::create_directory(root));
        fs::create_directory(root / "Assets");
    }
    void TearDown() override
    {
        // Delete only this fixture's explicitly created child of the temp directory.
        std::error_code ec;
        if (!root.empty() && fs::equivalent(root.parent_path(), fs::temp_directory_path(), ec) &&
            root.filename().string().starts_with("zengine-material-test-")) fs::remove_all(root, ec);
    }
    fs::path File(std::string const& name) const { return root / "Assets" / name; }
    static std::string Read(fs::path const& path)
    {
        std::ifstream in(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    }
    static void Write(fs::path const& path, std::string_view bytes)
    {
        std::ofstream out(path, std::ios::binary);
        out << bytes;
        ASSERT_TRUE(out.good());
    }
    static std::string Utf8(fs::path const& path)
    {
        auto value = path.u8string();
        return {reinterpret_cast<char const*>(value.data()), value.size()};
    }
    static RenderMaterialPtr Fresh(fs::path const& path) { return LoadRenderMaterialFile(path, false); }
    static void Equal(RenderMaterial const& a, RenderMaterial const& b)
    {
        EXPECT_EQ(a.Name(), b.Name());
        for (size_t i = 0; i < 4; ++i) EXPECT_EQ(a.Albedo()[i], b.Albedo()[i]);
        for (size_t i = 0; i < 3; ++i) EXPECT_EQ(a.Emissive()[i], b.Emissive()[i]);
        EXPECT_EQ(a.Metalness(), b.Metalness());
        EXPECT_EQ(a.Glossiness(), b.Glossiness());
        EXPECT_EQ(a.Transparent(), b.Transparent());
        EXPECT_EQ(a.AlphaTestThreshold(), b.AlphaTestThreshold());
        EXPECT_EQ(a.Sss(), b.Sss());
        EXPECT_EQ(a.TwoSided(), b.TwoSided());
        EXPECT_EQ(a.NormalScale(), b.NormalScale());
        EXPECT_EQ(a.OcclusionStrength(), b.OcclusionStrength());
        EXPECT_EQ(a.HeightOffset(), b.HeightOffset());
        EXPECT_EQ(a.HeightScale(), b.HeightScale());
        EXPECT_EQ(a.DetailMode(), b.DetailMode());
        EXPECT_EQ(a.EdgeTessHint(), b.EdgeTessHint());
        EXPECT_EQ(a.InsideTessHint(), b.InsideTessHint());
        EXPECT_EQ(a.MinTessFactor(), b.MinTessFactor());
        EXPECT_EQ(a.MaxTessFactor(), b.MaxTessFactor());
        for (int i = 0; i < RenderMaterial::TS_NumTextureSlots; ++i)
            EXPECT_EQ(a.TextureName(static_cast<RenderMaterial::TextureSlot>(i)), b.TextureName(static_cast<RenderMaterial::TextureSlot>(i)));
    }
};

TEST_F(MaterialIO, MetalnessGlossinessRoundTrip)
{
    for (float value : {0.f, 0.23f, 1.f})
    {
        auto material = std::make_shared<RenderMaterial>();
        material->Metalness(value);
        material->Glossiness(1.f - value);
        auto path = File(std::to_string(value) + ".mat");
        SaveRenderMaterial(material, path.string());
        auto loaded = Fresh(path);
        EXPECT_FLOAT_EQ(value, loaded->Metalness());
        EXPECT_FLOAT_EQ(1.f - value, loaded->Glossiness());
        EXPECT_EQ(std::string::npos, Read(path).find("metalness_glossiness value="));
    }
}

TEST_F(MaterialIO, IsolatedTempAssets)
{
    EXPECT_TRUE(fs::equivalent(fs::temp_directory_path(), root.parent_path()));
    EXPECT_TRUE(fs::is_directory(root / "Assets"));
    EXPECT_TRUE(fs::is_empty(root / "Assets"));
}

TEST_F(MaterialIO, AllEditableFieldsRoundTrip)
{
    auto m = std::make_shared<RenderMaterial>();
    m->Name("Material & \"demo\" <test>");
    m->Albedo(float4(0.12345679f, 0.00000001234f, 0.81f, 0.23f));
    m->Metalness(0.23456789f); m->Glossiness(0.7654321f);
    m->Emissive(float3(12.f, 0.0000000789f, 3.1234567f));
    m->NormalScale(0.42f); m->OcclusionStrength(0.64f);
    for (int i = 0; i < RenderMaterial::TS_NumTextureSlots; ++i)
        m->TextureName(static_cast<RenderMaterial::TextureSlot>(i), "Textures/map & " + std::to_string(i) + ".png");
    auto path = File("all.mat");
    SaveRenderMaterial(m, path.string());
    auto loaded = Fresh(path);
    Equal(*m, *loaded);
    // Clear every value and texture; old XML attributes must not resurrect them.
    auto zero = std::make_shared<RenderMaterial>();
    zero->Name("");
    for (int i = 0; i < RenderMaterial::TS_NumTextureSlots; ++i) loaded->TextureName(static_cast<RenderMaterial::TextureSlot>(i), "");
    loaded->Albedo(float4(0, 0, 0, 0)); loaded->Metalness(0); loaded->Glossiness(0);
    loaded->Emissive(float3(0, 0, 0)); loaded->NormalScale(0); loaded->OcclusionStrength(0); loaded->Name("");
    SaveRenderMaterial(loaded, path.string());
    Equal(*zero, *Fresh(path));
}

TEST_F(MaterialIO, PreservesUnexposedFieldsWithoutTextures)
{
    for (auto mode : {RenderMaterial::SurfaceDetailMode::ParallaxMapping, RenderMaterial::SurfaceDetailMode::ParallaxOcclusionMapping,
        RenderMaterial::SurfaceDetailMode::FlatTessellation, RenderMaterial::SurfaceDetailMode::SmoothTessellation})
    {
        auto m = std::make_shared<RenderMaterial>();
        m->Transparent(true); m->Sss(true); m->TwoSided(true); m->AlphaTestThreshold(0.3456789f);
        m->NormalScale(0.72f); m->OcclusionStrength(0.81f);
        m->HeightOffset(-0.12345679f); m->HeightScale(0.00000003456f);
        m->DetailMode(mode); m->EdgeTessHint(2.3f); m->InsideTessHint(3.4f); m->MinTessFactor(1.2f); m->MaxTessFactor(17.8f);
        SaveRenderMaterial(m, File("advanced.mat").string());
        auto loaded = Fresh(File("advanced.mat"));
        Equal(*m, *loaded);
        loaded->Transparent(false); loaded->Sss(false); loaded->TwoSided(false); loaded->AlphaTestThreshold(0);
        SaveRenderMaterial(loaded, File("advanced.mat").string());
        Equal(*loaded, *Fresh(File("advanced.mat")));
    }
}

TEST_F(MaterialIO, UnicodePathAndName)
{
    auto path = root / "Assets" / fs::path(u8"材质球.mat");
    auto m = std::make_shared<RenderMaterial>();
    m->Name(Utf8(fs::path(u8"金属材质")));
    m->TextureName(RenderMaterial::TS_Albedo, Utf8(fs::path(u8"Textures/颜色.png")));
    SaveRenderMaterial(m, Utf8(path));
    Equal(*m, *Fresh(path));
}

TEST_F(MaterialIO, FloatPrecisionIncludingSubnormals)
{
    for (auto value : {std::numeric_limits<float>::denorm_min(), std::numeric_limits<float>::min(),
        0.12345679f, std::numeric_limits<float>::max()})
    {
        auto m = std::make_shared<RenderMaterial>();
        m->NormalScale(value);
        SaveRenderMaterial(m, File("precision.mat").string());
        EXPECT_EQ(value, Fresh(File("precision.mat"))->NormalScale());
    }
}

TEST_F(MaterialIO, MissingAndMalformedFiles)
{
    EXPECT_THROW(Fresh(File("missing.mat")), std::runtime_error);
    EXPECT_THROW(SyncLoadRenderMaterial(File("runtime-missing.mat").string()), std::runtime_error);
    for (auto xml : {"", "   ", "<material>", "<material></other>", "<material/><material/>", "<wrong/>",
        "<material><albedo color=\"1 2\"/></material>", "<material><normal scale=\"no\"/></material>",
        "<material><normal scale=\"1junk\"/></material>", "<material><normal scale=\"nan\"/></material>",
        "<material><sss value=\"0.5\"/></material>", "<material><detail mode=\"future\"/></material>",
        "<material><normal scale=\"1\" scale=\"2\"/></material>", "<material><normal/><normal/></material>"})
    {
        SCOPED_TRACE(xml);
        Write(File("bad.mat"), xml);
        EXPECT_THROW(Fresh(File("bad.mat")), std::runtime_error);
    }
}

TEST_F(MaterialIO, LegacyAndUnknownFields)
{
    Write(File("legacy.mat"), R"(<material custom="keep"><metalness value="0.25"/><glossiness value="0.75"/>
        <normal scale="0.3" vendor="keep"><extension payload="keep"/></normal><vendor data="keep"/></material>)");
    auto m = Fresh(File("legacy.mat"));
    EXPECT_EQ("legacy", m->Name());
    EXPECT_EQ(0.25f, m->Metalness()); EXPECT_EQ(0.75f, m->Glossiness());
    m = m->Clone();
    m->Metalness(0.8f);
    SaveRenderMaterial(m, File("legacy.mat").string());
    auto xml = Read(File("legacy.mat"));
    for (auto expected : {"custom=\"keep\"", "vendor=\"keep\"", "payload=\"keep\"", "data=\"keep\""})
        EXPECT_NE(std::string::npos, xml.find(expected));
    EXPECT_EQ(0.8f, Fresh(File("legacy.mat"))->Metalness());
    Write(File("ambiguous.mat"), "<material><metalness_glossiness value=\"0.2\"/></material>");
    try { Fresh(File("ambiguous.mat")); FAIL() << "Ambiguous legacy data must not be guessed"; }
    catch (std::runtime_error const& e) { EXPECT_NE(std::string::npos, std::string(e.what()).find("Ambiguous")); }
    Write(File("minimal.mat"), "<material/>");
    EXPECT_EQ(1.f, Fresh(File("minimal.mat"))->NormalScale());
}

TEST_F(MaterialIO, FreshReadBypassesResourceCache)
{
    Write(File("fresh.mat"), "<material><metalness value=\"0.1\"/></material>");
    auto cached = SyncLoadRenderMaterial(File("fresh.mat").string());
    Write(File("fresh.mat"), "<material><metalness value=\"0.9\"/></material>");
    EXPECT_EQ(0.9f, Fresh(File("fresh.mat"))->Metalness());
    EXPECT_EQ(0.1f, cached->Metalness());
}

TEST_F(MaterialIO, FailureKeepsOriginal)
{
    using namespace RenderWorker::Detail;
    for (auto stage : {MaterialSaveStage::Open, MaterialSaveStage::Write, MaterialSaveStage::Close, MaterialSaveStage::Replace})
    {
        Write(File("target.mat"), "original bytes");
        EXPECT_THROW(WriteMaterialFileAtomically(File("target.mat"), "new material bytes", [stage](auto current) {
            if (current == stage) TMSG("injected IO failure");
        }), std::runtime_error);
        EXPECT_EQ("original bytes", Read(File("target.mat")));
        EXPECT_EQ(1, std::distance(fs::directory_iterator(root / "Assets"), fs::directory_iterator()));
    }
}

TEST_F(MaterialIO, SuccessReplacesFileAndLeavesNoTemporaryFiles)
{
    auto m = std::make_shared<RenderMaterial>();
    auto path = File("target.mat");
    SaveRenderMaterial(m, path.string());
    m->Metalness(0.67f);
    SaveRenderMaterial(m, path.string());
    EXPECT_EQ(0.67f, Fresh(path)->Metalness());
    EXPECT_EQ(1, std::distance(fs::directory_iterator(root / "Assets"), fs::directory_iterator()));
}

TEST_F(MaterialIO, SaveRejectsInvalidTargetAndData)
{
    auto m = std::make_shared<RenderMaterial>();
    Write(File("target.mat"), "original");
    EXPECT_THROW(SaveRenderMaterial({}, File("target.mat").string()), std::runtime_error);
    m->Metalness(std::numeric_limits<float>::infinity());
    EXPECT_THROW(SaveRenderMaterial(m, File("target.mat").string()), std::runtime_error);
    EXPECT_EQ("original", Read(File("target.mat")));
    m->Metalness(0);
    EXPECT_THROW(SaveRenderMaterial(m, (File("missing") / "target.mat").string()), std::exception);
    EXPECT_FALSE(fs::exists(File("missing")));
    EXPECT_THROW(SaveRenderMaterial(m, (root / "Assets").string()), std::runtime_error);
}

#if defined(_WIN32)
TEST_F(MaterialIO, RealReplaceFailureKeepsOriginal)
{
    Write(File("locked.mat"), "original");
    HANDLE lock = ::CreateFileW(File("locked.mat").c_str(), GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    ASSERT_NE(INVALID_HANDLE_VALUE, lock);
    EXPECT_THROW(SaveRenderMaterial(std::make_shared<RenderMaterial>(), File("locked.mat").string()), std::runtime_error);
    ::CloseHandle(lock);
    EXPECT_EQ("original", Read(File("locked.mat")));
    EXPECT_EQ(1, std::distance(fs::directory_iterator(root / "Assets"), fs::directory_iterator()));
}
#endif
}
