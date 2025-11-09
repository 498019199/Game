#include <base/ZEngine.h>
#include <audio/AudioFactory.h>

namespace RenderWorker
{
AudioFactory::AudioFactory() noexcept = default;
AudioFactory::~AudioFactory() noexcept = default;

AudioEngine& AudioFactory::AudioEngineInstance()
{
    if (!ae_)
    {
        ae_ = this->MakeAudioEngine();
    }

    return *ae_;
}

void AudioFactory::Suspend()
{
    if (ae_)
    {
        ae_->Suspend();
    }
    this->DoSuspend();
}

void AudioFactory::Resume()
{
    this->DoResume();
    if (ae_)
    {
        ae_->Resume();
    }
}

}