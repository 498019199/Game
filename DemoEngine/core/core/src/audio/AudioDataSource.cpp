#include <audio/AudioDataSource.h>

namespace RenderWorker
{
AudioDataSourceFactory::AudioDataSourceFactory() noexcept = default;
AudioDataSourceFactory::~AudioDataSourceFactory() noexcept = default;

AudioDataSource::AudioDataSource() noexcept = default;
AudioDataSource::~AudioDataSource() noexcept = default;

AudioFormat AudioDataSource::Format() const
{
    return format_;
}

uint32_t AudioDataSource::Freq() const
{
    return freq_;
}


void AudioDataSourceFactory::Suspend()
{
    this->DoSuspend();
}

void AudioDataSourceFactory::Resume()
{
    this->DoResume();
}

}
