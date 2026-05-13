#pragma once
#include <base/ZEngine.h>

namespace RenderWorker
{

enum AudioFormat
{
    AF_Mono8,
    AF_Mono16,
    AF_Stereo8,
    AF_Stereo16,

    AF_Unknown,
};

class ZENGINE_CORE_API AudioDataSource
{
    ZENGINE_NONCOPYABLE(AudioDataSource);

public:
    AudioDataSource() noexcept;
    virtual ~AudioDataSource() noexcept;

    virtual void Open(ResIdentifierPtr const & file) = 0;
    virtual void Close() = 0;

    AudioFormat Format() const;
    uint32_t Freq() const;

    virtual size_t Size() = 0;

    virtual size_t Read(void* data, size_t size) = 0;
    virtual void Reset() = 0;

protected:
    AudioFormat format_;
    uint32_t freq_;
};
using AudioDataSourcePtr = std::shared_ptr<AudioDataSource>;

class ZENGINE_CORE_API AudioDataSourceFactory
{
    ZENGINE_NONCOPYABLE(AudioDataSourceFactory);

public:
    AudioDataSourceFactory() noexcept;
    virtual ~AudioDataSourceFactory() noexcept;

    void Suspend();
    void Resume();

    virtual std::wstring const & Name() const = 0;

    virtual AudioDataSourcePtr MakeAudioDataSource() = 0;

private:
    virtual void DoSuspend() = 0;
    virtual void DoResume() = 0;
};

}