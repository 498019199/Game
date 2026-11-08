#include <base/ZEngine.h>
#include <base/AudioFactory.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <thread>

namespace RenderWorker
{
namespace
{
void Check(bool ok, char const* operation)
{
    if (!ok) TMSG(std::string(operation) + ": " + SDL_GetError());
}
AudioDataSourcePtr const& CheckedSource(AudioDataSourcePtr const& source)
{
    if (!source) TMSG("SDL3 audio requires a data source");
    return source;
}
SDL_AudioSpec Spec(AudioDataSource const& source)
{
    SDL_AudioSpec spec{};
    switch (source.Format()) {
    case AF_Mono8: spec.channels = 1; spec.format = SDL_AUDIO_U8; break;
    case AF_Mono16: spec.channels = 1; spec.format = SDL_AUDIO_S16; break;
    case AF_Stereo8: spec.channels = 2; spec.format = SDL_AUDIO_U8; break;
    case AF_Stereo16: spec.channels = 2; spec.format = SDL_AUDIO_S16; break;
    default: TMSG("Unsupported SDL3 audio PCM format");
    }
    if (source.Freq() == 0 || source.Freq() > static_cast<uint32_t>(std::numeric_limits<int>::max()))
        TMSG("Invalid SDL3 audio sample rate");
    spec.freq = static_cast<int>(source.Freq());
    return spec;
}
struct AudioSubsystem
{
    AudioSubsystem() { Check(SDL_InitSubSystem(SDL_INIT_AUDIO), "SDL_InitSubSystem(AUDIO)"); }
    ~AudioSubsystem() { SDL_QuitSubSystem(SDL_INIT_AUDIO); }
};
struct DestroyStream { void operator()(SDL_AudioStream* stream) const { SDL_DestroyAudioStream(stream); } };
class Stream
{
public:
    Stream(SDL_AudioSpec const& spec, SDL_AudioStreamCallback callback = nullptr, void* user = nullptr)
        : stream_(SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, callback, user))
    {
        Check(stream_ != nullptr, "SDL_OpenAudioDeviceStream");
    }
    SDL_AudioStream* Get() const { return stream_.get(); }
    void Volume(float value) {
        if (!std::isfinite(value)) TMSG("Invalid SDL3 audio volume");
        Check(SDL_SetAudioStreamGain(Get(), std::clamp(value, 0.0f, 1.0f)), "SDL_SetAudioStreamGain");
    }
private:
    AudioSubsystem audio_; // Initialized before the stream, released after it.
    std::unique_ptr<SDL_AudioStream, DestroyStream> stream_;
};

class SoundVoice
{
public:
    SoundVoice(SDL_AudioSpec const& spec, std::vector<uint8_t> const& pcm)
        : pcm_(pcm), frame_bytes_(SDL_AUDIO_FRAMESIZE(spec)), stream_(spec, Feed, this) {}
    ~SoundVoice() { SDL_SetAudioStreamGetCallback(stream_.Get(), nullptr, nullptr); }
    void Play(bool loop) {
        Check(SDL_PauseAudioStreamDevice(stream_.Get()), "SDL_PauseAudioStreamDevice");
        {
            Check(SDL_LockAudioStream(stream_.Get()), "SDL_LockAudioStream");
            auto unlock = CommonWorker::Defer([&] { SDL_UnlockAudioStream(stream_.Get()); });
            Check(SDL_ClearAudioStream(stream_.Get()), "SDL_ClearAudioStream");
            cursor_ = 0; loop_ = loop; started_ = true; eof_ = false;
        }
        Check(SDL_ResumeAudioStreamDevice(stream_.Get()), "SDL_ResumeAudioStreamDevice");
    }
    void Stop() {
        SDL_PauseAudioStreamDevice(stream_.Get());
        Check(SDL_LockAudioStream(stream_.Get()), "SDL_LockAudioStream");
        auto unlock = CommonWorker::Defer([&] { SDL_UnlockAudioStream(stream_.Get()); });
        started_ = false; cursor_ = 0; eof_ = false;
        SDL_ClearAudioStream(stream_.Get());
    }
    bool Playing() const {
        Check(SDL_LockAudioStream(stream_.Get()), "SDL_LockAudioStream");
        auto unlock = CommonWorker::Defer([&] { SDL_UnlockAudioStream(stream_.Get()); });
        return started_ && (!eof_ || SDL_GetAudioStreamQueued(stream_.Get()) > 0 || SDL_GetAudioStreamAvailable(stream_.Get()) > 0);
    }
    void Volume(float value) { stream_.Volume(value); }
private:
    static void SDLCALL Feed(void* user, SDL_AudioStream* stream, int needed, int) noexcept {
        auto& voice = *static_cast<SoundVoice*>(user);
        if (!voice.started_ || voice.eof_ || needed <= 0) return;
        size_t remaining = (static_cast<size_t>(needed) + voice.frame_bytes_ - 1) / voice.frame_bytes_ * voice.frame_bytes_;
        while (remaining > 0) {
            size_t const bytes = std::min(remaining, voice.pcm_.size() - voice.cursor_);
            if (!SDL_PutAudioStreamData(stream, voice.pcm_.data() + voice.cursor_, static_cast<int>(bytes))) {
                voice.started_ = false; return;
            }
            voice.cursor_ += bytes; remaining -= bytes;
            if (voice.cursor_ == voice.pcm_.size()) {
                if (voice.loop_) voice.cursor_ = 0;
                else { voice.eof_ = true; SDL_FlushAudioStream(stream); break; }
            }
        }
    }
    std::vector<uint8_t> const& pcm_;
    size_t frame_bytes_;
    Stream stream_;
    size_t cursor_ = 0;
    bool started_ = false, eof_ = false, loop_ = false; // Protected by SDL's stream lock.
};

class SDL3SoundBuffer final : public SoundBuffer
{
public:
    SDL3SoundBuffer(AudioDataSourcePtr const& source, uint32_t voices, float volume)
        : SoundBuffer(CheckedSource(source))
    {
        if (voices == 0 || voices > 64) TMSG("SDL3 sound voice count must be between 1 and 64");
        auto const spec = Spec(*source);
        source->Reset();
        std::array<uint8_t, 16384> chunk;
        while (size_t const n = source->Read(chunk.data(), chunk.size())) {
            if (n > chunk.size()) TMSG("Audio source returned an invalid byte count");
            pcm_.insert(pcm_.end(), chunk.begin(), chunk.begin() + n);
        }
        if (pcm_.empty() || pcm_.size() % SDL_AUDIO_FRAMESIZE(spec) != 0) TMSG("Empty or incomplete SDL3 sound PCM data");
        for (uint32_t i = 0; i < voices; ++i) voices_.push_back(std::make_unique<SoundVoice>(spec, pcm_));
        Volume(volume);
    }
    void Play(bool loop) override {
        for (auto& voice : voices_) if (!voice->Playing()) { voice->Play(loop); return; }
        voices_[next_voice_]->Play(loop); next_voice_ = (next_voice_ + 1) % voices_.size();
    }
    void Stop() override { for (auto& voice : voices_) voice->Stop(); }
    bool IsPlaying() const override { return std::any_of(voices_.begin(), voices_.end(), [](auto const& v) { return v->Playing(); }); }
    void Volume(float value) override { for (auto& voice : voices_) voice->Volume(value); }
    float3 Position() const override { return position_; }
    void Position(float3 const& v) override { position_ = v; }
    float3 Velocity() const override { return velocity_; }
    void Velocity(float3 const& v) override { velocity_ = v; }
    float3 Direction() const override { return direction_; }
    void Direction(float3 const& v) override { direction_ = v; }
private:
    void DoReset() override { next_voice_ = 0; }
    // SDL streams provide 2D playback; spatial state is retained for future 3D mixing.
    float3 position_{0,0,0}, velocity_{0,0,0}, direction_{0,0,1};
    std::vector<uint8_t> pcm_;
    std::vector<std::unique_ptr<SoundVoice>> voices_;
    size_t next_voice_ = 0;
};

class SDL3MusicBuffer final : public MusicBuffer
{
public:
    SDL3MusicBuffer(AudioDataSourcePtr const& source, uint32_t seconds, float volume)
        : MusicBuffer(CheckedSource(source)), stream_(Spec(*source))
    {
        auto const spec = Spec(*source);
        target_bytes_ = static_cast<int>(std::min<uint64_t>(uint64_t(spec.freq) * SDL_AUDIO_FRAMESIZE(spec) * std::clamp(seconds, 1u, 60u),
            std::numeric_limits<int>::max() / 2));
        Volume(volume);
    }
    ~SDL3MusicBuffer() noexcept override { DoStop(); }
    bool IsPlaying() const override { return playing_.load(); }
    void Volume(float value) override { stream_.Volume(value); }
    float3 Position() const override { return position_; }
    void Position(float3 const& v) override { position_ = v; }
    float3 Velocity() const override { return velocity_; }
    void Velocity(float3 const& v) override { velocity_ = v; }
    float3 Direction() const override { return direction_; }
    void Direction(float3 const& v) override { direction_ = v; }
private:
    void DoReset() override { data_source_->Reset(); }
    void DoPlay(bool loop) override {
        playing_ = true;
        try {
            Check(SDL_ResumeAudioStreamDevice(stream_.Get()), "SDL_ResumeAudioStreamDevice");
            worker_ = std::jthread([this, loop](std::stop_token stop) {
                try {
                    std::array<uint8_t, 16384> chunk;
                    bool eof = false;
                    while (!stop.stop_requested()) {
                        int const queued = SDL_GetAudioStreamQueued(stream_.Get());
                        Check(queued >= 0, "SDL_GetAudioStreamQueued");
                        if (eof) {
                            if (queued == 0 && SDL_GetAudioStreamAvailable(stream_.Get()) == 0) break;
                        } else if (queued < target_bytes_) {
                            size_t n = data_source_->Read(chunk.data(), chunk.size());
                            if (n == 0 && loop) { data_source_->Reset(); n = data_source_->Read(chunk.data(), chunk.size()); }
                            if (n > chunk.size()) TMSG("Audio source returned an invalid byte count");
                            if (n == 0) { Check(SDL_FlushAudioStream(stream_.Get()), "SDL_FlushAudioStream"); eof = true; }
                            else { Check(SDL_PutAudioStreamData(stream_.Get(), chunk.data(), static_cast<int>(n)), "SDL_PutAudioStreamData"); continue; }
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    }
                } catch (std::exception const& e) { LogError() << "SDL3 music playback failed: " << e.what() << std::endl; }
                playing_ = false;
            });
        } catch (...) { playing_ = false; SDL_PauseAudioStreamDevice(stream_.Get()); throw; }
    }
    void DoStop() override {
        // Join before MusicBuffer::Stop/Play resets the decoder shared with this worker.
        if (worker_.joinable()) { worker_.request_stop(); worker_.join(); }
        SDL_PauseAudioStreamDevice(stream_.Get());
        SDL_ClearAudioStream(stream_.Get());
        playing_ = false;
    }
    float3 position_{0,0,0}, velocity_{0,0,0}, direction_{0,0,1};
    Stream stream_;
    std::jthread worker_;
    std::atomic<bool> playing_{false};
    int target_bytes_ = 0;
};

class SDL3AudioEngine final : public AudioEngine
{
public:
    std::wstring const& Name() const override { static std::wstring const name = L"SDL3 Audio Engine"; return name; }
    float3 GetListenerPos() const override { return position_; }
    void SetListenerPos(float3 const& v) override { position_ = v; }
    float3 GetListenerVel() const override { return velocity_; }
    void SetListenerVel(float3 const& v) override { velocity_ = v; }
    void GetListenerOri(float3& face, float3& up) const override { face = face_; up = up_; }
    void SetListenerOri(float3 const& face, float3 const& up) override { face_ = face; up_ = up; }
private:
    void DoSuspend() override {}
    void DoResume() override {}
    float3 position_{0,0,0}, velocity_{0,0,0}, face_{0,0,1}, up_{0,1,0};
};
}
}
extern "C" ZENGINE_SYMBOL_EXPORT void MakeAudioFactory(std::unique_ptr<RenderWorker::AudioFactory>& ptr)
{
    ptr = std::make_unique<RenderWorker::ConcreteAudioFactory<RenderWorker::SDL3AudioEngine,
        RenderWorker::SDL3SoundBuffer, RenderWorker::SDL3MusicBuffer>>(L"SDL3 Audio Factory");
}
