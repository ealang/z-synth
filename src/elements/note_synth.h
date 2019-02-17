#ifndef NOTE_SYNTH_H
#define NOTE_SYNTH_H

#include <cstdint> 

class NoteSynth {
  const uint32_t sampleRateHz;
  const uint32_t channelCount;
  const float velocity;

  bool off = false;
  uint32_t sampleCount = 0;
  uint32_t offSample = 0;

  float phase = 0;
  const float periodSize;
  const float stepSize;
  const uint32_t attackSampleSize;
  const uint32_t releaseSampleSize;
  const uint32_t decaySampleSize;

public:
  NoteSynth(
    uint32_t sampleRateHz,
    uint32_t channelCount,
    float freqHz,
    float velocity
  );

  bool isExhausted();
  void generate(uint32_t nSamples, float* buffer);
  void postOffEvent();
  void postPressureEvent(float pressure);
};

#endif
