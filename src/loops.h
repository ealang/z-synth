#ifndef LOOPS_H
#define LOOPS_H

#include <cstdint> 
#include <memory>
#include <mutex>
#include <alsa/asoundlib.h>
#include "./pipeline/pipeline_element.h"

class MidiMux;

struct AudioParam {
  const uint32_t sampleRateHz;
  const uint32_t bufferSampleCount;
  const uint32_t channelCount;
};

int audioLoop(snd_pcm_t *const audioDevice, std::mutex& lock, std::shared_ptr<AudioElement<float>> pipeline, AudioParam audioParam);
int midiLoop(snd_seq_t *const midiDevice, std::mutex& lock, std::shared_ptr<MidiListener> pipeline);

#endif
