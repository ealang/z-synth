#ifndef LOOPS_H
#define LOOPS_H

#include <cstdint> 
#include <alsa/asoundlib.h>

class MidiMux;

struct AudioParam {
  const uint32_t sampleRateHz;
  const uint32_t bufferSampleCount;
  const uint32_t channelCount;
};

int audioLoop(snd_pcm_t *const audioDevice, MidiMux *const mux, AudioParam *const audioParam);
int midiLoop(snd_seq_t *const midiDevice, MidiMux *const mux);

#endif
