#include <base/ZEngine.h>
#include <base/AudioFactory.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <fstream>
#include <thread>

using namespace RenderWorker;
using namespace std::chrono_literals;

namespace
{
// Silent PCM exercises the actual playback device without producing test noise.
class SilentSource final : public AudioDataSource
{
public:
    explicit SilentSource(size_t bytes = 4410) : bytes_(bytes) { format_ = AF_Mono16; freq_ = 44100; }
    void Open(ResIdentifierPtr const&) override {}
    void Close() override {}
    size_t Size() override { return bytes_; }
    size_t Read(void* data, size_t capacity) override {
        reading = true;
        std::this_thread::sleep_for(2ms);
        size_t const n = std::min(capacity, bytes_ - cursor_);
        std::memset(data, 0, n);
        cursor_ += n;
        ++reads;
        reading = false;
        return n;
    }
    void Reset() override {
        if (reading.load()) reset_during_read = true;
        cursor_ = 0;
        ++resets;
    }
    std::atomic<unsigned> reads{0}, resets{0};
    std::atomic<bool> reading{false}, reset_during_read{false};
private:
    size_t bytes_, cursor_ = 0;
};

template<class Predicate> bool WaitFor(Predicate predicate)
{
    auto const deadline = std::chrono::steady_clock::now() + 3s;
    while (!predicate()) {
        if (std::chrono::steady_clock::now() >= deadline) return false;
        std::this_thread::sleep_for(5ms);
    }
    return true;
}

class CoreAudio : public testing::Test
{
protected:
    void SetUp() override {
        Context::Destroy();
        auto& context = Context::Instance();
        context.LoadConfig("KlayGE.cfg");
        auto config = context.Config();
        config.audio_factory_name = "CoreAudio";
        context.Config(config);
        factory = &context.AudioFactoryInstance();
    }
    void TearDown() override { factory = nullptr; Context::Destroy(); }
    AudioFactory* factory = nullptr;
};
}

TEST_F(CoreAudio, MusicCompletesLoopsAndStopsBeforeResettingDecoder)
{
    auto source = std::make_shared<SilentSource>();
    auto music = factory->MakeMusicBuffer(source, 1);
    music->Play(false);
    EXPECT_TRUE(music->IsPlaying());
    ASSERT_TRUE(WaitFor([&] { return !music->IsPlaying(); }));
    EXPECT_GT(source->reads.load(), 1u);
    auto const resets = source->resets.load();
    music->Play(true);
    ASSERT_TRUE(WaitFor([&] { return source->resets.load() > resets + 1; }));
    EXPECT_TRUE(music->IsPlaying());
    music->Stop();
    EXPECT_FALSE(music->IsPlaying());
    for (unsigned i = 0; i < 5; ++i) {
        music->Play(true);
        ASSERT_TRUE(WaitFor([&] { return source->reading.load(); }));
        music->Stop();
    }
    EXPECT_FALSE(source->reset_during_read.load());
    music->Reset();
    music->Play(false);
    EXPECT_TRUE(WaitFor([&] { return !music->IsPlaying(); }));
}

TEST_F(CoreAudio, SoundPreloadsPcmAndSupportsPolyphonyLoopAndReplay)
{
    auto source = std::make_shared<SilentSource>();
    auto sound = factory->MakeSoundBuffer(source, 2);
    auto const reads = source->reads.load();
    EXPECT_GT(reads, 1u);
    sound->Play(false);
    sound->Play(false);
    EXPECT_TRUE(sound->IsPlaying());
    ASSERT_TRUE(WaitFor([&] { return !sound->IsPlaying(); }));
    sound->Volume(0.25f);
    sound->Play(true);
    std::this_thread::sleep_for(120ms);
    EXPECT_TRUE(sound->IsPlaying());
    sound->Stop();
    EXPECT_FALSE(sound->IsPlaying());
    sound->Reset();
    sound->Play(false);
    ASSERT_TRUE(WaitFor([&] { return !sound->IsPlaying(); }));
    EXPECT_EQ(source->reads.load(), reads);
}

TEST_F(CoreAudio, EmptyLoopTerminatesAndInvalidBuffersAreRejected)
{
    auto empty = std::make_shared<SilentSource>(0);
    auto music = factory->MakeMusicBuffer(empty);
    music->Play(true);
    ASSERT_TRUE(WaitFor([&] { return !music->IsPlaying(); }));
    EXPECT_THROW(factory->MakeSoundBuffer(empty), std::runtime_error);
    EXPECT_THROW(factory->MakeMusicBuffer(nullptr), std::runtime_error);
    EXPECT_THROW(factory->MakeSoundBuffer(nullptr), std::runtime_error);
    EXPECT_THROW(factory->MakeSoundBuffer(std::make_shared<SilentSource>(), 0), std::runtime_error);
}

TEST_F(CoreAudio, ConfigSupportsAutoExplicitAndLegacyBackends)
{
    auto& context = Context::Instance();
    std::ifstream original(context.ResLoaderInstance().Locate("KlayGE.cfg"));
    ASSERT_TRUE(original);
    std::string const xml{std::istreambuf_iterator<char>(original), std::istreambuf_iterator<char>()};
    std::string const marker = "<audio_factory name=\"auto\"/>";
    auto const offset = xml.find(marker);
    ASSERT_NE(offset, std::string::npos);
    auto const directory = std::filesystem::temp_directory_path() / ("editor-audio-config-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    ASSERT_TRUE(std::filesystem::create_directory(directory));
    auto cleanup = CommonWorker::Defer([&] { std::error_code ec; std::filesystem::remove_all(directory, ec); });
    for (auto name : {"auto", "XAudio", "CoreAudio", "unavailable"}) {
        SCOPED_TRACE(name);
        auto copy = xml;
        copy.replace(offset, marker.size(), std::string("<audio_factory name=\"") + name + "\"/>");
        auto const file = directory / (std::string(name) + ".cfg");
        { std::ofstream output(file); output << copy; ASSERT_TRUE(output); }
        context.LoadConfig(file.string().c_str());
        EXPECT_EQ("CoreAudio", context.Config().audio_factory_name);
    }
}
