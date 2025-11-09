#include <base/ZEngine.h>
#include <audio/Audio.h>

namespace RenderWorker
{
AudioBuffer::AudioBuffer(AudioDataSourcePtr const & data_source)
    : data_source_(data_source),
        format_(data_source_->Format()),
        freq_(data_source_->Freq())
{
}

AudioBuffer::~AudioBuffer() noexcept = default;

void AudioBuffer::Suspend()
{
    if (this->IsPlaying())
    {
        resume_playing_ = true;
        this->Stop();
    }
}

void AudioBuffer::Resume()
{
    if (resume_playing_)
    {
        this->Play();
        resume_playing_ = false;
    }
}
}
