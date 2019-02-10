#ifndef NOTE_SYNTH_H
#define NOTE_SYNTH_H

#include <cstdint> 

class NoteSynth {
  uint32_t sampleRateHz;
  uint32_t channelCount;
  float freqHz;
  float velocity;

  bool off = false;
  uint32_t sampleCount = 0;
  uint32_t offSample = 0;
  uint32_t phase = 0;

  uint32_t periodSize;
  uint32_t attackSampleSize;
  uint32_t releaseSampleSize;
  uint32_t decaySampleSize;

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
