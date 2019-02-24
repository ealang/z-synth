#ifndef ALSA_H
#define ALSA_H

#include <tuple>
#include <functional>
#include <alsa/asoundlib.h>
#include "./audio_params.h"

class CLIParams;

using sample_t = int16_t;

snd_seq_t* openMidiDevice(const CLIParams& params);
void pollMidiEvents(snd_seq_t *const midiDevice, std::function<void(const snd_seq_event_t*)> onEvent);

std::pair<snd_pcm_t*, AudioParams> openAudioDevice(const CLIParams& params);
void writeAudioFrames(snd_pcm_t* audioDevice, uint32_t numFrames, sample_t *data);

#endif
