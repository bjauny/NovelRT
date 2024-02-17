// Copyright © Matt Jones and Contributors. Licensed under the MIT Licence (MIT). See LICENCE.md in the repository root
// for more information.

#include <NovelRT/Ecs/Ecs.h>

namespace NovelRT::Ecs::Audio
{
    AudioSystem::AudioSystem(
        std::shared_ptr<PluginManagement::IResourceManagementPluginProvider> resourceManagerPluginProvider)
        : _counter(1),
          _logger(Utilities::Misc::CONSOLE_LOG_AUDIO),
          _mixer(std::make_shared<NovelRT::Audio::AudioMixer>()),
          _soundCache(std::vector<uint32_t>()),
          _systemTime(Timing::Timestamp::zero()),
          _resourceManagerPluginProvider(std::move(resourceManagerPluginProvider))
    {
        _mixer->Initialise();
    }

    void AudioSystem::Update(Timing::Timestamp delta, Ecs::Catalogue catalogue)
    {
        _systemTime += delta;
        auto [emitters, states] = catalogue.GetComponentViews<AudioEmitterComponent, AudioEmitterStateComponent>();

        for (auto [entity, emitterState] : states)
        {
            auto emitter = emitters.GetComponent(entity);

            switch (emitterState.state)
            {
                case AudioEmitterState::ToPlay:
                {
                    _mixer->PlaySource(emitter.handle);
                    _logger.logDebug("Entity ID {} - EmitterState ToPlay -> Playing", entity);
                    states.PushComponentUpdateInstruction(
                        entity, AudioEmitterStateComponent{AudioEmitterState::Playing, emitterState.fadeDuration});
                    break;
                }
                case AudioEmitterState::ToStop:
                {
                    _mixer->StopSource(emitter.handle);
                    _logger.logDebug("Entity ID {} - EmitterState ToStop -> Stopped", entity);
                    states.PushComponentUpdateInstruction(
                        entity, AudioEmitterStateComponent{AudioEmitterState::Stopped, emitterState.fadeDuration});
                    break;
                }
                case AudioEmitterState::ToPause:
                {
                    _mixer->PauseSource(emitter.handle);
                    _logger.logDebug("Entity ID {} - EmitterState ToPause -> Paused", entity);
                    states.PushComponentUpdateInstruction(
                        entity, AudioEmitterStateComponent{AudioEmitterState::Paused, emitterState.fadeDuration});
                    break;
                }
                case AudioEmitterState::ToResume:
                {
                    _mixer->PlaySource(emitter.handle);
                    _logger.logDebug("Entity ID {} - EmitterState ToResume -> Playing", entity);
                    states.PushComponentUpdateInstruction(
                        entity, AudioEmitterStateComponent{AudioEmitterState::Playing, emitterState.fadeDuration});
                    break;
                }
                // case AudioEmitterState::ToFadeOut:
                // {
                //     if (emitter.isMusic && !_mixer->IsMusicPlaying())
                //     {
                //         states.PushComponentUpdateInstruction(
                //             entity, AudioEmitterStateComponent{AudioEmitterState::Stopped, 0.0f, 0.0f});
                //         break;
                //     }
                //     else if (!emitter.isMusic && !_mixer->IsSoundPlaying(_soundCache.at(emitter.handle)))
                //     {
                //         states.PushComponentUpdateInstruction(
                //             entity, AudioEmitterStateComponent{AudioEmitterState::Stopped, 0.0f, 0.0f});
                //         break;
                //     }

                //     float slope = -(emitter.volume / emitterState.fadeDuration);
                //     Timing::Timestamp endTime = _systemTime + Timing::Timestamp::fromSeconds(emitterState.fadeDuration);
                //     _fadeCache.insert({entity, std::make_tuple(endTime, slope)});

                //     states.PushComponentUpdateInstruction(
                //         entity,
                //         AudioEmitterStateComponent{AudioEmitterState::FadingOut, emitterState.fadeDuration, 0.0f});
                //     _logger.logDebug("Entity ID {} - EmitterState ToFadeOut -> FadingOut", entity);
                //     break;
                // }
                // case AudioEmitterState::FadingOut:
                // {
                //     Timing::Timestamp endTime = std::get<0>(_fadeCache.at(entity));
                //     float remainingDuration = (endTime - _systemTime).getSecondsFloat();

                //     if (_systemTime < endTime)
                //     {
                //         float slope = std::get<1>(_fadeCache.at(entity));
                //         float newVolume = emitter.volume + (slope * delta.getSecondsFloat());
                //         ChangeAudioVolume(emitter, newVolume);
                //         states.PushComponentUpdateInstruction(
                //             entity, AudioEmitterStateComponent{AudioEmitterState::FadingOut, remainingDuration,
                //                                                emitterState.fadeExpectedVolume});
                //         emitters.PushComponentUpdateInstruction(
                //             entity,
                //             AudioEmitterComponent{emitter.handle, emitter.isMusic, emitter.numberOfLoops, newVolume});
                //         _logger.logDebug("Entity ID {} - EmitterState FadingOut at volume {}, slope: {}, currentTime: "
                //                          "{}, remaining: {}",
                //                          entity, newVolume, slope, _systemTime.getSecondsFloat(), remainingDuration);
                //         break;
                //     }
                //     else
                //     {
                //         ChangeAudioVolume(emitter, 0.0f);
                //         if (emitter.isMusic)
                //         {
                //             _mixer->StopMusic();
                //         }
                //         else
                //         {
                //             _mixer->StopSound(_soundCache.at(emitter.handle));
                //         }

                //         emitters.PushComponentUpdateInstruction(
                //             entity, AudioEmitterComponent{emitter.handle, emitter.isMusic, emitter.numberOfLoops,
                //                                           emitterState.fadeExpectedVolume});
                //         states.PushComponentUpdateInstruction(
                //             entity, AudioEmitterStateComponent{AudioEmitterState::Stopped, 0.0f, 0.0f});
                //         _fadeCache.erase(entity);
                //         _logger.logDebug("Entity ID {} - EmitterState FadingOut -> Stopped", entity);
                //         break;
                //     }

                //     break;
                // }
                // case AudioEmitterState::ToFadeIn:
                // {
                //     if (emitter.isMusic && !_mixer->IsMusicPlaying())
                //     {
                //         _mixer->SetMusicVolume(0.0f);
                //         _mixer->PlayMusic(_musicCache.at(emitter.handle), emitter.numberOfLoops);
                //     }
                //     else if (!emitter.isMusic && !_mixer->IsSoundPlaying(emitter.handle))
                //     {
                //         auto sound = _soundCache.at(emitter.handle);
                //         _mixer->SetSoundVolume(sound, 0.0f);
                //         _mixer->PlaySound(sound, emitter.numberOfLoops);
                //     }
                //     else
                //     {
                //         states.PushComponentUpdateInstruction(
                //             entity, AudioEmitterStateComponent{AudioEmitterState::Playing, 0.0f, 0.0f});
                //         break;
                //     }

                //     float slope = (emitterState.fadeExpectedVolume / emitterState.fadeDuration);
                //     Timing::Timestamp endTime = _systemTime + Timing::Timestamp::fromSeconds(emitterState.fadeDuration);
                //     _fadeCache.insert({entity, std::make_tuple(endTime, slope)});

                //     states.PushComponentUpdateInstruction(
                //         entity, AudioEmitterStateComponent{AudioEmitterState::FadingIn, emitterState.fadeDuration,
                //                                            emitterState.fadeExpectedVolume});
                //     emitters.PushComponentUpdateInstruction(
                //         entity, AudioEmitterComponent{emitter.handle, emitter.isMusic, emitter.numberOfLoops, 0.0f});
                //     _logger.logDebug("Entity ID {} - EmitterState ToFadeIn -> FadingIn", entity);
                //     break;
                // }
                // case AudioEmitterState::FadingIn:
                // {
                //     Timing::Timestamp endTime = std::get<0>(_fadeCache.at(entity));
                //     float remainingDuration = (endTime - _systemTime).getSecondsFloat();

                //     if (_systemTime < endTime)
                //     {
                //         float slope = std::get<1>(_fadeCache.at(entity));
                //         float newVolume = emitter.volume + (slope * delta.getSecondsFloat());
                //         ChangeAudioVolume(emitter, newVolume);
                //         states.PushComponentUpdateInstruction(
                //             entity, AudioEmitterStateComponent{AudioEmitterState::FadingIn, remainingDuration,
                //                                                emitterState.fadeExpectedVolume});
                //         emitters.PushComponentUpdateInstruction(
                //             entity,
                //             AudioEmitterComponent{emitter.handle, emitter.isMusic, emitter.numberOfLoops, newVolume});
                //         _logger.logDebug("Entity ID {} - EmitterState FadingIn at volume {}, slope: {}, currentTime: "
                //                          "{}, remaining: {}",
                //                          entity, newVolume, slope, _systemTime.getSecondsFloat(), remainingDuration);
                //         break;
                //     }
                //     else
                //     {
                //         if (emitter.volume < emitterState.fadeExpectedVolume)
                //         {
                //             ChangeAudioVolume(emitter, emitterState.fadeExpectedVolume);
                //         }
                //         emitters.PushComponentUpdateInstruction(
                //             entity, AudioEmitterComponent{emitter.handle, emitter.isMusic, emitter.numberOfLoops,
                //                                           emitterState.fadeExpectedVolume});
                //         states.PushComponentUpdateInstruction(
                //             entity, AudioEmitterStateComponent{AudioEmitterState::Playing, 0.0f, 0.0f});
                //         _fadeCache.erase(entity);
                //         _logger.logDebug("Entity ID {} - EmitterState FadingIn -> Playing", entity);
                //         break;
                //     }
                // }
                case AudioEmitterState::Playing:
                {
                    auto soundContext = _mixer->GetSourceContext(emitter.handle);
                    if (soundContext.Volume != emitter.volume)
                    {
                        _mixer->SetSourceVolume(emitter.handle, emitter.volume);
                        _logger.logDebug("Entity ID {} - Emitter Volume {} -> {}", entity, soundContext.Volume,
                                            emitter.volume);
                    }
                    if (_mixer->GetSourceState(emitter.handle) != NovelRT::Audio::AudioSourceState::SOURCE_PLAYING)
                    {
                        states.PushComponentUpdateInstruction(
                            entity, AudioEmitterStateComponent{AudioEmitterState::ToStop, 0.0f, 0.0f});
                    }
                    break;
                }
                case AudioEmitterState::Stopped:
                case AudioEmitterState::Paused:
                default:
                {
                    break;
                }
            }
        }

        //_mixer->CheckSources();
    }

    uint32_t AudioSystem::RegisterSound(std::string fileName)
    {
        if (_counter == UINT32_MAX)
        {
            return 0U;
        }

        auto asset = _resourceManagerPluginProvider->GetResourceLoader()->LoadAudioFrameData(fileName);
        auto handle = _mixer->SubmitAudioBuffer(asset.processedAudioFrames);
        // if (_mixer->IsLoaded(handle))
        // {
        //     _soundCache.insert({_counter, handle});
        //     value = _counter;
        _counter++;
        // }
        return handle;
    }

    // void AudioSystem::ChangeAudioVolume(AudioEmitterComponent emitter, float desiredVolume)
    // {
    //     if (emitter.isMusic)
    //     {
    //         _mixer->SetMusicVolume(desiredVolume);
    //     }
    //     else
    //     {
    //         _mixer->SetSoundVolume(_soundCache.at(emitter.handle), desiredVolume);
    //     }
    // }

    AudioSystem::~AudioSystem() noexcept
    {
        unused(_soundCache.empty());
    }
}
