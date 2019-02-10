#ifndef LOOPS_H
#define LOOPS_H

#include <cstdint> 
#include <alsa/asoundlib.h>
#include <memory>
#include "./pipeline/pipeline_element.h"

class MidiMux;

struct AudioParam {
  const uint32_t sampleRateHz;
  const uint32_t bufferSampleCount;
  const uint32_t channelCount;
};

int audioLoop(snd_pcm_t *const audioDevice, std::shared_ptr<MidiAudioElement<float>> pipeline, AudioParam audioParam);
int midiLoop(snd_seq_t *const midiDevice, std::shared_ptr<MidiAudioElement<float>> pipeline);

#endif
