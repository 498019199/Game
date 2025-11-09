#include <base/ZEngine.h>
#include <audio/Audio.h>

namespace RenderWorker
{
MusicBuffer::MusicBuffer(AudioDataSourcePtr const & data_source)
    : AudioBuffer(data_source)
{
}

MusicBuffer::~MusicBuffer() noexcept = default;

bool MusicBuffer::IsSound() const
{
    return false;
}

void MusicBuffer::Reset()
{
    this->Stop();

    this->DoReset();
}

void MusicBuffer::Play(bool loop)
{
    this->DoStop();
    this->DoPlay(loop);
}

void MusicBuffer::Stop()
{
    if (this->IsPlaying())
    {
        this->DoStop();
        data_source_->Reset();
    }
}
}