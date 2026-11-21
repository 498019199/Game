#include <base/AudioFactory.h>
#include <AudioToolbox/AudioToolbox.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <mutex>

namespace RenderWorker
{
namespace
{
void Check(OSStatus status, char const *operation)
{
    if (status != noErr)
        TMSG(std::string(operation) + " failed (" + std::to_string(status) + ")");
}

AudioDataSourcePtr const &CheckedSource(AudioDataSourcePtr const &source)
{
    if (!source || !source->Freq())
        TMSG("Core Audio requires a valid PCM source");
    return source;
}

AudioStreamBasicDescription Format(AudioDataSource const &source)
{
    AudioStreamBasicDescription format{};
    format.mSampleRate = source.Freq();
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kAudioFormatFlagIsPacked;
    switch (source.Format())
    {
    case AF_Mono8:
        format.mChannelsPerFrame = 1;
        format.mBitsPerChannel = 8;
        break;
    case AF_Stereo8:
        format.mChannelsPerFrame = 2;
        format.mBitsPerChannel = 8;
        break;
    case AF_Mono16:
        format.mChannelsPerFrame = 1;
        format.mBitsPerChannel = 16;
        break;
    case AF_Stereo16:
        format.mChannelsPerFrame = 2;
        format.mBitsPerChannel = 16;
        break;
    default:
        TMSG("Unsupported PCM format");
    }
    if (format.mBitsPerChannel == 16)
        format.mFormatFlags |= kAudioFormatFlagIsSignedInteger;
    format.mFramesPerPacket = 1;
    format.mBytesPerPacket = format.mBytesPerFrame = format.mChannelsPerFrame * format.mBitsPerChannel / 8;
    return format;
}

// Each voice has its own queue and reader. Queue callbacks and initial priming
// serialize access to the decoder, including a loop that rewinds at EOF.
class Voice
{
  public:
    using Reader = std::function<size_t(void *, size_t)>;
    Voice(AudioStreamBasicDescription const &format, UInt32 bytes, Reader read, std::function<void()> rewind)
        : read_(std::move(read)), rewind_(std::move(rewind)), frame_size_(format.mBytesPerFrame)
    {
        Check(AudioQueueNewOutput(&format, OnBuffer, this, nullptr, nullptr, 0, &queue_), "AudioQueueNewOutput");
        try
        {
            for (auto &buffer : buffers_)
                Check(AudioQueueAllocateBuffer(queue_, bytes, &buffer), "AudioQueueAllocateBuffer");
        }
        catch (...)
        {
            AudioQueueDispose(queue_, true);
            throw;
        }
    }

    ~Voice()
    {
        active_ = false;
        AudioQueueDispose(queue_, true);
    }
    bool Playing() const
    {
        return active_;
    }
    void Volume(float value)
    {
        if (!std::isfinite(value))
            TMSG("Invalid volume");
        Check(AudioQueueSetParameter(queue_, kAudioQueueParam_Volume, std::clamp(value, 0.f, 1.f)),
              "AudioQueueSetParameter");
    }
    void Stop()
    {
        active_ = false;
        Check(AudioQueueStop(queue_, true), "AudioQueueStop");
        std::lock_guard lock(lock_);
        pending_ = 0;
    }
    void Play(bool loop)
    {
        Stop();
        {
            std::lock_guard lock(lock_);
            rewind_();
            loop_ = loop;
            active_ = true;
            for (auto buffer : buffers_)
                if (!Fill(buffer))
                    break;
            if (!pending_)
            {
                active_ = false;
                return;
            }
        }
        try
        {
            Check(AudioQueueStart(queue_, nullptr), "AudioQueueStart");
        }
        catch (...)
        {
            Stop();
            throw;
        }
    }

  private:
    bool Fill(AudioQueueBufferRef buffer)
    {
        auto const cap = buffer->mAudioDataBytesCapacity;
        size_t n = read_(buffer->mAudioData, cap);
        if (!n && loop_)
        {
            rewind_();
            n = read_(buffer->mAudioData, cap);
        }
        if (!n)
            return false;
        if (n > cap || n % frame_size_)
            TMSG("Invalid PCM data size");
        buffer->mAudioDataByteSize = static_cast<UInt32>(n);
        Check(AudioQueueEnqueueBuffer(queue_, buffer, 0, nullptr), "AudioQueueEnqueueBuffer");
        ++pending_;
        return true;
    }
    static void OnBuffer(void *user, AudioQueueRef, AudioQueueBufferRef buffer) noexcept
    {
        auto &voice = *static_cast<Voice *>(user);
        std::lock_guard lock(voice.lock_);
        if (!voice.active_)
            return;
        --voice.pending_;
        try
        {
            if (!voice.Fill(buffer) && voice.pending_ == 0)
                voice.active_ = false;
        }
        catch (std::exception const &e)
        {
            LogError() << "Core Audio playback failed: " << e.what() << std::endl;
            voice.active_ = false;
        }
    }

    Reader read_;
    std::function<void()> rewind_;
    UInt32 frame_size_;
    AudioQueueRef queue_{};
    std::array<AudioQueueBufferRef, 3> buffers_{};
    std::mutex lock_;
    unsigned pending_{};
    bool loop_{};
    std::atomic<bool> active_{false};
};

class CoreAudioSoundBuffer final : public SoundBuffer
{
  public:
    CoreAudioSoundBuffer(AudioDataSourcePtr const &source, uint32_t count, float volume)
        : SoundBuffer(CheckedSource(source))
    {
        if (count == 0 || count > 64)
            TMSG("Sound voice count must be between 1 and 64");
        auto format = Format(*source);
        source->Reset();
        std::array<uint8_t, 16384> chunk;
        while (size_t n = source->Read(chunk.data(), chunk.size()))
        {
            if (n > chunk.size())
                TMSG("Invalid audio source read");
            pcm_.insert(pcm_.end(), chunk.begin(), chunk.begin() + n);
        }
        if (pcm_.empty() || pcm_.size() % format.mBytesPerFrame)
            TMSG("Invalid sound PCM");
        offsets_.resize(count);
        for (size_t i = 0; i < count; ++i)
        {
            voices_.push_back(std::make_unique<Voice>(
                format, 16384,
                [this, i](void *dst, size_t size) {
                    size = std::min(size, pcm_.size() - offsets_[i]);
                    std::memcpy(dst, pcm_.data() + offsets_[i], size);
                    offsets_[i] += size;
                    return size;
                },
                [this, i] { offsets_[i] = 0; }));
        }
        Volume(volume);
    }
    void Play(bool loop) override
    {
        for (auto &voice : voices_)
            if (!voice->Playing())
            {
                voice->Play(loop);
                return;
            }
        voices_[next_voice_]->Play(loop);
        next_voice_ = (next_voice_ + 1) % voices_.size();
    }
    void Stop() override
    {
        for (auto &voice : voices_)
            voice->Stop();
    }
    bool IsPlaying() const override
    {
        return std::any_of(voices_.begin(), voices_.end(), [](auto const &v) { return v->Playing(); });
    }
    void Volume(float value) override
    {
        for (auto &voice : voices_)
            voice->Volume(value);
    }
    float3 Position() const override
    {
        return position_;
    }
    void Position(float3 const &v) override
    {
        position_ = v;
    }
    float3 Velocity() const override
    {
        return velocity_;
    }
    void Velocity(float3 const &v) override
    {
        velocity_ = v;
    }
    float3 Direction() const override
    {
        return direction_;
    }
    void Direction(float3 const &v) override
    {
        direction_ = v;
    }

  private:
    void DoReset() override
    {
        next_voice_ = 0;
    }
    float3 position_{0, 0, 0}, velocity_{0, 0, 0}, direction_{0, 0, 1};
    std::vector<uint8_t> pcm_;
    std::vector<size_t> offsets_;
    std::vector<std::unique_ptr<Voice>> voices_;
    size_t next_voice_ = 0;
};

class CoreAudioMusicBuffer final : public MusicBuffer
{
  public:
    CoreAudioMusicBuffer(AudioDataSourcePtr const &source, uint32_t seconds, float volume)
        : MusicBuffer(CheckedSource(source)),
          voice_(
              Format(*source),
              static_cast<UInt32>(
                  std::clamp<uint64_t>(uint64_t(source->Freq()) * std::clamp(seconds, 1u, 60u) / 3, 1, 65536)) *
                  Format(*source).mBytesPerFrame,
              [source](void *dst, size_t cap) { return source->Read(dst, cap); }, [source] { source->Reset(); })
    {
        Volume(volume);
    }
    bool IsPlaying() const override
    {
        return voice_.Playing();
    }
    void Volume(float value) override
    {
        voice_.Volume(value);
    }
    float3 Position() const override
    {
        return position_;
    }
    void Position(float3 const &v) override
    {
        position_ = v;
    }
    float3 Velocity() const override
    {
        return velocity_;
    }
    void Velocity(float3 const &v) override
    {
        velocity_ = v;
    }
    float3 Direction() const override
    {
        return direction_;
    }
    void Direction(float3 const &v) override
    {
        direction_ = v;
    }

  private:
    void DoReset() override
    {
        data_source_->Reset();
    }
    void DoStop() override
    {
        voice_.Stop();
    }
    void DoPlay(bool loop) override
    {
        voice_.Play(loop);
    }
    float3 position_{0, 0, 0}, velocity_{0, 0, 0}, direction_{0, 0, 1};
    Voice voice_;
};

class CoreAudioAudioEngine final : public AudioEngine
{
  public:
    std::wstring const &Name() const override
    {
        static std::wstring const name = L"CoreAudio Audio Engine";
        return name;
    }
    float3 GetListenerPos() const override
    {
        return position_;
    }
    void SetListenerPos(float3 const &v) override
    {
        position_ = v;
    }
    float3 GetListenerVel() const override
    {
        return velocity_;
    }
    void SetListenerVel(float3 const &v) override
    {
        velocity_ = v;
    }
    void GetListenerOri(float3 &face, float3 &up) const override
    {
        face = face_;
        up = up_;
    }
    void SetListenerOri(float3 const &face, float3 const &up) override
    {
        face_ = face;
        up_ = up;
    }

  private:
    void DoSuspend() override
    {
    }
    void DoResume() override
    {
    }
    float3 position_{0, 0, 0}, velocity_{0, 0, 0}, face_{0, 0, 1}, up_{0, 1, 0};
};
}
}
extern "C" ZENGINE_SYMBOL_EXPORT void MakeAudioFactory(std::unique_ptr<RenderWorker::AudioFactory> &ptr)
{
    ptr = std::make_unique<RenderWorker::ConcreteAudioFactory<
        RenderWorker::CoreAudioAudioEngine, RenderWorker::CoreAudioSoundBuffer, RenderWorker::CoreAudioMusicBuffer>>(
        L"CoreAudio Audio Factory");
}
