#include <base/ZEngine.h>
#include <game/SceneAssets.h>
#include <gtest/gtest.h>
#include <chrono>
#include <fstream>
using namespace CommonWorker;
namespace
{
JsonValue Object() { return JsonValue(JsonValueType::Object); }
TEST(SceneMaterials, ReferencesAreIndependentAndUnknownFieldsSurvive)
{
    auto entry = Object(); entry.AppendValue("NpcId", JsonValue(300001)); entry.AppendValue("Custom", JsonValue(std::string_view("preserve")));
    SceneAssets::Set(entry,0,0,"materials/red.mat"); SceneAssets::Set(entry,1,0,"materials/blue.mat");
    auto copy = entry; SceneAssets::Set(copy,0,0,"materials/green.mat");
    auto original = SceneAssets::Read(entry), changed = SceneAssets::Read(copy);
    ASSERT_EQ(2u,changed.size()); EXPECT_EQ("materials/red.mat",original[0].asset);
    EXPECT_EQ("materials/green.mat",changed[0].asset); EXPECT_EQ("materials/blue.mat",changed[1].asset);
    EXPECT_EQ("preserve",copy.Member("Custom")->ValueString()); EXPECT_EQ(300001,copy.Member("NpcId")->ValueInt());
}
TEST(SceneMaterials, LegacyPrefabStringUpgradesWithoutLosingItsReference)
{
    JsonValue entry(std::string_view("prefabs/test.prefab"));
    SceneAssets::Set(entry,0,3,"materials/a.mat");
    EXPECT_EQ("prefabs/test.prefab",entry.Member("Prefab")->ValueString());
    auto refs = SceneAssets::Read(entry); ASSERT_EQ(1u,refs.size()); EXPECT_EQ(3u,refs[0].slot);
}
TEST(SceneMaterials, RejectsMalformedAndDuplicateReferences)
{
    auto entry = Object(); entry.AppendValue("MaterialOverrides",JsonValue(42));
    EXPECT_THROW(SceneAssets::Read(entry),std::runtime_error);
    entry = Object(); SceneAssets::Set(entry,0,0,"materials/a.mat");
    auto array = entry.Member("MaterialOverrides"); auto duplicate = array->ValueArray()[0]; array->AppendValue(duplicate);
    EXPECT_THROW(SceneAssets::Read(entry),std::runtime_error);
    entry = Object(); SceneAssets::Set(entry,0,0,"materials/a.mat");
    JsonValue item(entry.Member("MaterialOverrides")->ValueArray()[0]); item.Member("Slot")->Value(-1);
    entry.Member("MaterialOverrides")->ValueIndex(0,std::move(item));
    EXPECT_THROW(SceneAssets::Read(entry),std::runtime_error);
}
TEST(SceneMaterials, ReplacingAnAssetPreservesReferenceMetadata)
{
    auto entry = Object(); SceneAssets::Set(entry,0,0,"materials/a.mat");
    auto array = entry.Member("MaterialOverrides"); auto item = array->ValueArray()[0];
    item.AppendValue("FutureProperty",JsonValue(true)); array->ValueIndex(0,std::move(item));
    SceneAssets::Set(entry,0,0,"materials/b.mat");
    EXPECT_TRUE(entry.Member("MaterialOverrides")->ValueArray()[0].Member("FutureProperty")->ValueBool());
}
TEST(SceneMaterials, PathsMustNameExistingMaterialsInsideAssets)
{
    namespace fs = std::filesystem;
    auto root = fs::temp_directory_path() / ("scene-material-paths-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directory(root); fs::create_directory(root/"assets"); fs::create_directory(root/"assets/materials");
    struct Cleanup { fs::path path; ~Cleanup() { std::error_code error; fs::remove_all(path,error); } } cleanup{root};
    std::ofstream(root/"assets/materials/a.mat") << "test"; std::ofstream(root/"outside.mat") << "test";
    EXPECT_EQ(fs::canonical(root/"assets/materials/a.mat"),SceneAssets::Resolve(root/"assets","materials/a.mat"));
    for (auto path : {"../outside.mat","materials/missing.mat","/outside.mat","C:/outside.mat","materials/a.txt","materials\\a.mat"})
        EXPECT_THROW(SceneAssets::Resolve(root/"assets",path),std::runtime_error) << path;
    std::error_code error; fs::create_symlink(root/"outside.mat",root/"assets/link.mat",error);
    if (!error) EXPECT_THROW(SceneAssets::Resolve(root/"assets","link.mat"),std::runtime_error);
}
}
