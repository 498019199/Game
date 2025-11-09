#include <base/ZEngine.h>
#include <audio/Audio.h>

namespace RenderWorker
{
AudioEngine::AudioEngine() = default;
AudioEngine::~AudioEngine() noexcept = default;

void AudioEngine::Suspend()
{
    for (auto const & ab : audio_buffs_)
    {
        ab.second->Suspend();
    }
    this->DoSuspend();
}

void AudioEngine::Resume()
{
    this->DoResume();
    for (auto const & ab : audio_buffs_)
    {
        ab.second->Resume();
    }
}

void AudioEngine::AddBuffer(size_t id, AudioBufferPtr const & buffer)
{
    audio_buffs_.emplace(id, buffer);
}

void AudioEngine::Play(size_t buf_id, bool loop)
{
    this->Buffer(buf_id)->Play(loop);
}

void AudioEngine::Stop(size_t buf_id)
{
    this->Buffer(buf_id)->Stop();
}

void AudioEngine::PlayAll(bool loop)
{
    for (auto const & ab : audio_buffs_)
    {
        ab.second->Play(loop);
    }
}

void AudioEngine::StopAll()
{
    for (auto const & ab : audio_buffs_)
    {
        ab.second->Stop();
    }
}

size_t AudioEngine::NumBuffer() const
{
    return audio_buffs_.size();
}

AudioBufferPtr AudioEngine::Buffer(size_t buff_id) const
{
    auto iter = audio_buffs_.find(buff_id);
    if (iter != audio_buffs_.end())
    {
        return iter->second;
    }

    ZENGINE_UNREACHABLE("Invalid buffer id");
}

void AudioEngine::SoundVolume(float vol)
{
    sound_vol_ = vol;

    for (auto const & ab : audio_buffs_)
    {
        if (ab.second->IsSound())
        {
            ab.second->Volume(vol);
        }
    }
}

float AudioEngine::SoundVolume() const
{
    return sound_vol_;
}

void AudioEngine::MusicVolume(float vol)
{
    music_vol_ = vol;

    for (auto const & ab : audio_buffs_)
    {
        if (!(ab.second->IsSound()))
        {
            ab.second->Volume(vol);
        }
    }
}

float AudioEngine::MusicVolume() const
{
    return music_vol_;
}
}