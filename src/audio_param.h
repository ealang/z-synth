#ifndef TYPES_H
#define TYPES_H

#include <cstdint> 
typedef int16_t sample_t;

struct AudioParam {
  const uint32_t sampleRateHz;
  const uint32_t bufferSampleCount;
  const uint32_t channelCount;
};

#endif
