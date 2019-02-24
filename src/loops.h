#ifndef LOOPS_H
#define LOOPS_H

#include <functional>

#include "./alsa.h"
#include "./audio_params.h"

int midiLoop(
  snd_seq_t* const midiDevice,
  std::function<void(const snd_seq_event_t*)> onEvent
);

int audioLoop(
  snd_pcm_t* audioDevice,
  AudioParams audioParam,
  std::function<void(float*)> onSample
);

#endif
