#ifndef LOOPS_H
#define LOOPS_H

#include <cstdint> 
#include <functional>
#include <alsa/asoundlib.h>

using sample_t = int16_t;

struct AudioParam {
  const uint32_t sampleRateHz;
  const uint32_t bufferSampleCount;
  const uint32_t channelCount;
};

int audioLoop(
  snd_pcm_t* audioDevice,
  AudioParam audioParam,
  std::function<void(sample_t*)> onSample
);

int midiLoop(
  snd_seq_t *const midiDevice,
  std::function<void(const snd_seq_event_t*)> onEvent
);

#endif
