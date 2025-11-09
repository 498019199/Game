#pragma once
#include <audio/AudioDataSource.h>

namespace RenderWorker
{
class ZENGINE_CORE_API AudioBuffer
{
    ZENGINE_NONCOPYABLE(AudioBuffer);

public:
    explicit AudioBuffer(AudioDataSourcePtr const & data_source);
    virtual ~AudioBuffer() noexcept;

    void Suspend();
    void Resume();

    virtual void Play(bool loop = false) = 0;
    virtual void Reset() = 0;
    virtual void Stop() = 0;

    virtual void Volume(float vol) = 0;

    virtual bool IsPlaying() const = 0;
    virtual bool IsSound() const = 0;

    virtual float3 Position() const = 0;
    virtual void Position(float3 const & v) = 0;
    virtual float3 Velocity() const = 0;
    virtual void Velocity(float3 const & v) = 0;
    virtual float3 Direction() const = 0;
    virtual void Direction(float3 const & v) = 0;

protected:
    AudioDataSourcePtr data_source_;

    AudioFormat	format_;
    uint32_t freq_;

    bool resume_playing_{false};
};

using AudioBufferPtr = std::shared_ptr<AudioBuffer>;

class ZENGINE_CORE_API SoundBuffer : public AudioBuffer
{
public:
    explicit SoundBuffer(AudioDataSourcePtr const & data_source);
    ~SoundBuffer() noexcept override;

    void Reset() override;

    bool IsSound() const override;

protected:
    virtual void DoReset() = 0;
};

class ZENGINE_CORE_API MusicBuffer : public AudioBuffer
{
public:
    explicit MusicBuffer(AudioDataSourcePtr const & data_source);
    ~MusicBuffer() noexcept override;

    void Play(bool loop = false) override;
    void Stop() override;
    void Reset() override;

    bool IsSound() const override;

protected:
    virtual void DoReset() = 0;
    virtual void DoPlay(bool loop) = 0;
    virtual void DoStop() = 0;

    static uint32_t constexpr BUFFERS_PER_SECOND = 2;
};

class ZENGINE_CORE_API AudioEngine
{
    ZENGINE_NONCOPYABLE(AudioEngine);

public:
    AudioEngine();
    virtual ~AudioEngine() noexcept;

    void Suspend();
    void Resume();

    virtual std::wstring const & Name() const = 0;

    virtual void AddBuffer(size_t id, AudioBufferPtr const & buffer);

    size_t NumBuffer() const;
    virtual AudioBufferPtr Buffer(size_t buff_id) const;

    void Play(size_t buff_id, bool loop = false);
    void Stop(size_t buff_id);
    void PlayAll(bool loop = false);
    void StopAll();

    void  SoundVolume(float vol);
    float SoundVolume() const;
    void  MusicVolume(float vol);
    float MusicVolume() const;

    virtual float3 GetListenerPos() const = 0;
    virtual void SetListenerPos(float3 const & v) = 0;
    virtual float3 GetListenerVel() const = 0;
    virtual void SetListenerVel(float3 const & v) = 0;
    virtual void GetListenerOri(float3& face, float3& up) const = 0;
    virtual void SetListenerOri(float3 const & face, float3 const & up) = 0;

private:
    virtual void DoSuspend() = 0;
    virtual void DoResume() = 0;

protected:
    std::map<size_t, AudioBufferPtr> audio_buffs_;

    float sound_vol_{1};
    float music_vol_{1};
};
}
