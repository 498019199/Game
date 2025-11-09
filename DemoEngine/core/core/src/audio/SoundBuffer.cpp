#include <base/ZEngine.h>
#include <audio/Audio.h>

namespace RenderWorker
{
SoundBuffer::SoundBuffer(AudioDataSourcePtr const & data_source)
    : AudioBuffer(data_source)
{
}

SoundBuffer::~SoundBuffer() noexcept = default;

bool SoundBuffer::IsSound() const
{
    return true;
}

void SoundBuffer::Reset()
{
    this->Stop();

    data_source_->Reset();

    this->DoReset();
}
}