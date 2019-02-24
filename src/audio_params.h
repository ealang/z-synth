#ifndef AUDIO_PARAM_H
#define AUDIO_PARAM_H

#include <cstdint> 

struct AudioParams {
  const uint32_t sampleRateHz;
  const uint32_t bufferSampleCount;
  const uint32_t channelCount;
};

#endif
