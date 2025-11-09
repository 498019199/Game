#pragma once
#include <base/ZEngine.h>
#include <audio/Audio.h>

namespace RenderWorker
{
class ZENGINE_CORE_API AudioFactory
{
    ZENGINE_NONCOPYABLE(AudioFactory);

public:
    AudioFactory() noexcept;
    virtual ~AudioFactory() noexcept;

    virtual std::wstring const & Name() const = 0;

    AudioEngine& AudioEngineInstance();

    void Suspend();
    void Resume();

    virtual AudioBufferPtr MakeSoundBuffer(AudioDataSourcePtr const & dataSource, uint32_t numSource = 1) = 0;
    virtual AudioBufferPtr MakeMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds = 2) = 0;

private:
    virtual std::unique_ptr<AudioEngine> MakeAudioEngine() = 0;
    virtual void DoSuspend() = 0;
    virtual void DoResume() = 0;

private:
    std::unique_ptr<AudioEngine> ae_;
};

template <typename AudioEngineType, typename SoundBufferType, typename MusicBufferType>
class ConcreteAudioFactory : public AudioFactory
{
public:
    explicit ConcreteAudioFactory(std::wstring const & name)
        : name_(name)
    {
    }

    std::wstring const & Name() const override
    {
        return name_;
    }

    AudioBufferPtr MakeSoundBuffer(AudioDataSourcePtr const & dataSource, uint32_t numSource = 1) override
    {
        return MakeSharedPtr<SoundBufferType>(dataSource, numSource,
            this->AudioEngineInstance().SoundVolume());
    }

    AudioBufferPtr MakeMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds = 2) override
    {
        return MakeSharedPtr<MusicBufferType>(dataSource, bufferSeconds,
            this->AudioEngineInstance().MusicVolume());
    }

private:
    std::unique_ptr<AudioEngine> MakeAudioEngine() override
    {
        return MakeUniquePtr<AudioEngineType>();
    }

    virtual void DoSuspend() override
    {
    }
    virtual void DoResume() override
    {
    }

private:
    std::wstring const name_;
};
}