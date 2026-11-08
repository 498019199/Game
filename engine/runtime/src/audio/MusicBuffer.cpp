#include <base/ZEngine.h>
#include <base/Audio.h>

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
    data_source_->Reset();
    this->DoPlay(loop);
}

void MusicBuffer::Stop()
{
    // Queued GPU/audio buffers are not a reliable indication of worker lifetime.
    this->DoStop();
    data_source_->Reset();
}
}
