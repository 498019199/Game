#include <base/ZEngine.h>
#include <base/Audio.h>
#include <base/AudioFactory.h>
#include <base/App3D.h>
#include <base/Window.h>
#include <base/UIManager.h>
#include <base/InputFactory.h>
#include <editor/EditorTransform.h>
#include <editor/EditorPanel.h>
#include <editor/EditorAssetScanner.h>
#include <fstream>
#include <chrono>
#include <gtest/gtest.h>

using namespace RenderWorker;
using namespace EditorWorker;

namespace
{
void ExpectMatrixNear(float4x4 const& lhs, float4x4 const& rhs)
{
    for (size_t row = 0; row < 4; ++row)
        for (size_t col = 0; col < 4; ++col)
            EXPECT_NEAR(lhs(row, col), rhs(row, col), 2e-4f);
}

class PreviewSource : public AudioDataSource
{
public:
    PreviewSource() { format_ = AF_Mono16; freq_ = 44100; }
    bool worker_active = false;
    int resets = 0;
    void Open(ResIdentifierPtr const&) override {}
    void Close() override {}
    size_t Size() override { return 0; }
    size_t Read(void*, size_t) override { return 0; }
    void Reset() override { EXPECT_FALSE(worker_active); ++resets; }
};

class PreviewBuffer : public MusicBuffer
{
public:
    explicit PreviewBuffer(std::shared_ptr<PreviewSource> source) : MusicBuffer(source), source_(source) {}
    // Simulate the race window before the worker queues its first audio packet.
    bool IsPlaying() const override { return false; }
    void Volume(float) override {}
    float3 Position() const override { return float3::Zero(); }
    void Position(float3 const&) override {}
    float3 Velocity() const override { return float3::Zero(); }
    void Velocity(float3 const&) override {}
    float3 Direction() const override { return float3::Zero(); }
    void Direction(float3 const&) override {}
protected:
    void DoReset() override { source_->Reset(); }
    void DoPlay(bool) override { source_->worker_active = true; }
    void DoStop() override { source_->worker_active = false; }
private:
    std::shared_ptr<PreviewSource> source_;
};
}

TEST(EditorTransform, EulerRoundTripIncludingGimbalLock)
{
    for (auto const degrees : {float3(90, 0, 0), float3(-90, 45, 20), float3(90, 45, 20), float3(25, 110, -35)})
    {
        auto rotation = MathWorker::rotation_matrix_yaw_pitch_roll(
            MathWorker::Deg2Rad(degrees.y()), MathWorker::Deg2Rad(degrees.x()), MathWorker::Deg2Rad(degrees.z()));
        auto angles = InspectorEulerDegrees(rotation);
        auto rebuilt = MathWorker::rotation_matrix_yaw_pitch_roll(
            MathWorker::Deg2Rad(angles.y()), MathWorker::Deg2Rad(angles.x()), MathWorker::Deg2Rad(angles.z()));
        ExpectMatrixNear(rotation, rebuilt);
    }
}

TEST(EditorTransform, PositionAndScalePreserveRotatedBasis)
{
    auto rotation = MathWorker::rotation_matrix_yaw_pitch_roll(0.7f, 1.57079632679f, -0.4f);
    float3 const scale(2, 3, 4);
    auto original = MathWorker::scaling(scale) * rotation * MathWorker::translation(7.0f, 8.0f, 9.0f);
    auto moved = InspectorTransform(original, float3(10, 11, 12), float3::Zero(), scale, scale, false, false);
    for (size_t row = 0; row < 3; ++row)
        for (size_t col = 0; col < 3; ++col)
            EXPECT_EQ(original(row, col), moved(row, col));
    auto resized = InspectorTransform(moved, float3(10, 11, 12), float3::Zero(), float3(4, 6, 8), scale, false, true);
    ExpectMatrixNear(resized, MathWorker::scaling(4.0f, 6.0f, 8.0f) * rotation * MathWorker::translation(10.0f, 11.0f, 12.0f));
}

TEST(EditorTransform, RotationEditPreservesPositionAndScale)
{
    float3 const scale(2, 3, 4);
    auto changed = InspectorTransform(float4x4::Identity(), float3(7, 8, 9), float3(30, 40, 50), scale, scale, true, false);
    float3 extracted_scale, extracted_position;
    quater rotation;
    MathWorker::decompose(extracted_scale, rotation, extracted_position, changed);
    for (size_t i = 0; i < 3; ++i)
        EXPECT_NEAR(scale[i], extracted_scale[i], 1e-5f);
    EXPECT_EQ(float3(7, 8, 9), extracted_position);
}

TEST(EditorAudio, StopAndReplayJoinBeforeResetEvenWithoutQueuedAudio)
{
    auto source = std::make_shared<PreviewSource>();
    PreviewBuffer buffer(source);
    buffer.Play();
    EXPECT_TRUE(source->worker_active);
    buffer.Play();
    buffer.Stop();
    EXPECT_FALSE(source->worker_active);
    EXPECT_EQ(3, source->resets);
}

TEST(EditorAudio, SelectionOwnsAndStopsItsPreview)
{
    auto source = std::make_shared<PreviewSource>();
    auto buffer = std::make_shared<PreviewBuffer>(source);
    {
        AssetAudioInfo selection;
        selection.audio_buff_ = source;
        selection.preview_buffer_ = buffer;
        buffer->Play();
    }
    EXPECT_FALSE(source->worker_active);
}

TEST(EditorRender, MissingPluginThrows)
{
    auto& context = Context::Instance();
    context.LoadConfig("missing-editor-regression.cfg");
    auto cfg = context.Config();
    cfg.render_factory_name = "MissingEditorTest";
    context.Config(cfg);
    EXPECT_THROW(context.RenderFactoryInstance(), std::runtime_error);
    Context::Destroy();
}

TEST(EditorAudio, MissingPluginThrows)
{
    auto& context = Context::Instance();
    context.ResLoaderInstance();
    context.LoadConfig("missing-editor-regression.cfg");
    auto cfg = context.Config();
    cfg.audio_factory_name = "MissingEditorTest";
    cfg.audio_data_source_factory_name = "MissingEditorTest";
    context.Config(cfg);
    EXPECT_THROW(context.AudioFactoryInstance(), std::runtime_error);
    EXPECT_THROW(context.AudioDataSourceFactoryInstance(), std::runtime_error);
    Context::Destroy();
}

TEST(EditorAudio, DefaultConfigLoadsBuiltFactoriesAndRejectsMissingResource)
{
    auto& context = Context::Instance();
    context.LoadConfig("KlayGE.cfg");
    EXPECT_EQ("XAudio", context.Config().audio_factory_name);
    EXPECT_EQ("OggVorbis", context.Config().audio_data_source_factory_name);
    EXPECT_NO_THROW(context.AudioFactoryInstance());
    auto source = context.AudioDataSourceFactoryInstance().MakeAudioDataSource();
    ASSERT_TRUE(source);
    EXPECT_THROW(source->Open(nullptr), std::runtime_error);
}

class EditorAssetScan : public testing::Test
{
protected:
    std::filesystem::path root;
    void SetUp() override
    {
        auto const base = std::filesystem::temp_directory_path();
        for (int attempt = 0; attempt < 100; ++attempt)
        {
            auto candidate = base / ("editor-scan-test-" + std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count()) + "-" + std::to_string(attempt));
            if (std::filesystem::create_directory(candidate))
            {
                root = candidate;
                return;
            }
        }
        FAIL() << "Could not create isolated scan fixture";
    }
    void TearDown() override
    {
        // Only remove the freshly created fixture, never a caller-supplied path.
        if (!root.empty() && root.parent_path() == std::filesystem::temp_directory_path() &&
            root.filename().string().find("editor-scan-test-") == 0)
        {
            std::error_code ec;
            std::filesystem::remove_all(root, ec);
        }
    }
};

TEST_F(EditorAssetScan, MissingDirectoryAndRegularFileAreSafe)
{
    EditorAssetScanner scanner;
    EXPECT_TRUE(scanner.ReadDirectory(root / "missing").empty());
    std::ofstream(root / "file.txt") << "hello";
    EXPECT_TRUE(scanner.ReadDirectory(root / "file.txt").empty());
}

TEST_F(EditorAssetScan, EnumeratesDirectoriesAndFileSizes)
{
    std::filesystem::create_directory(root / "nested");
    std::ofstream(root / "file.txt") << "hello";
    EditorAssetScanner scanner;
    auto entries = scanner.ReadDirectory(root);
    ASSERT_EQ(2u, entries.size());
    for (auto const& entry : entries)
    {
        if (entry.directory)
            EXPECT_EQ("nested", entry.path.filename().string());
        else
        {
            EXPECT_EQ("file.txt", entry.path.filename().string());
            EXPECT_EQ(5u, entry.size);
        }
    }
}

TEST_F(EditorAssetScan, CanonicalPathsAreVisitedOnce)
{
    std::ofstream(root / "file.txt") << "hello";
    EditorAssetScanner scanner;
    ASSERT_EQ(1u, scanner.ReadDirectory(root).size());
    EXPECT_TRUE(scanner.ReadDirectory(root / ".").empty());
    // A new scan can see the directory again.
    EXPECT_EQ(1u, EditorAssetScanner().ReadDirectory(root).size());
}

TEST_F(EditorAssetScan, DirectoryDeletedAfterEnumerationIsSafe)
{
    std::filesystem::create_directory(root / "gone");
    EditorAssetScanner scanner;
    auto entries = scanner.ReadDirectory(root);
    ASSERT_EQ(1u, entries.size());
    std::filesystem::remove(root / "gone");
    EXPECT_TRUE(scanner.ReadDirectory(entries.front().path).empty());
}

TEST_F(EditorAssetScan, SkipsAncestorAndDanglingLinks)
{
    std::filesystem::create_directory(root / "nested");
    std::error_code ec;
    std::filesystem::create_directory_symlink(root, root / "nested" / "parent", ec);
    if (ec)
        GTEST_SKIP() << "Directory symlink creation unavailable: " << ec.message();
    std::filesystem::create_directory_symlink(root / "missing", root / "dangling", ec);
    ASSERT_FALSE(ec) << ec.message();
    EditorAssetScanner scanner;
    EXPECT_EQ(1u, scanner.ReadDirectory(root).size());
    EXPECT_TRUE(scanner.ReadDirectory(root / "nested").empty());
    EXPECT_TRUE(scanner.ReadDirectory(root / "nested" / "parent").empty());
}

namespace
{
class InputTestApp : public App3D
{
public:
    InputTestApp() : App3D("Input routing regression") {}
private:
    uint32_t DoUpdate(uint32_t) override { return 0; }
};
}

TEST(InputRouting, EditorPriorityGamePassthroughAndFocusLoss)
{
    Context::Instance().LoadConfig("KlayGE.cfg");
    auto cfg = Context::Instance().Config();
    cfg.graphics_cfg.hide_win = true;
    Context::Instance().Config(cfg);
    InputTestApp app;
    ASSERT_NE(nullptr, app.MainWnd()->GetSDLWindow());
    app.MainWnd()->Active(true);
    auto& ui = Context::Instance().UIManagerInstance();
    ui.SetInputViewport(false, false, 0, 0, 640, 480);
    ui.RouteInput(true);
    EXPECT_TRUE(ui.GameKeyboardBlocked());
    EXPECT_TRUE(ui.GamePointerBlocked());
    ui.AcknowledgeGameInput();
    ui.SetInputViewport(true, true, 0, 0, 640, 480);
    ui.RouteInput(true);
    EXPECT_FALSE(ui.GameKeyboardBlocked());
    EXPECT_FALSE(ui.GamePointerBlocked());
    ui.RouteInput(true, true);
    EXPECT_TRUE(ui.GameKeyboardBlocked());
    EXPECT_TRUE(ui.GamePointerBlocked());
    ui.AcknowledgeGameInput();
    ui.SetInputViewport(false, false, 0, 0, 1, 1);
    ui.RouteInput(false);
    EXPECT_FALSE(ui.GameKeyboardBlocked());
    EXPECT_FALSE(ui.GamePointerBlocked());
    app.MainWnd()->Active(false);
    ui.RouteInput(false);
    EXPECT_TRUE(ui.GameKeyboardBlocked());
    EXPECT_TRUE(ui.GamePointerBlocked());
}

TEST(InputRouting, SDL3FactoryLoadsKeyboardAndMouse)
{
    Context::Instance().LoadConfig("KlayGE.cfg");
    EXPECT_EQ("SDL3", Context::Instance().Config().input_factory_name);
    auto& input = Context::Instance().InputFactoryInstance().InputEngineInstance();
    input.EnumDevices();
    ASSERT_GE(input.NumDevices(), 2u);
    EXPECT_EQ(InputEngine::IDT_Keyboard, input.Device(0)->Type());
    EXPECT_EQ(InputEngine::IDT_Mouse, input.Device(1)->Type());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    int const result = RUN_ALL_TESTS();
    Context::Destroy();
    return result;
}
