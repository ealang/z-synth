#ifndef NOTE_SYNTH_H
#define NOTE_SYNTH_H

#include <cstdint> 
#include "audio_param.h"

class NoteSynth {
  AudioParam audioParam;
  float freqHz;
  float velocity;

  double phase = 0;
  bool off = false;
  uint64_t sampleCount = 0;
  uint64_t offSample = 0;

  uint64_t attackSampleSize;
  uint64_t releaseSampleSize;
  uint64_t decaySampleSize;

public:
  NoteSynth(
      AudioParam audioParam,
      float freqHz,
      float velocity
  );

  bool isExhausted();
  void generate(uint64_t nSamples, sample_t* buffer);
  void postOffEvent();
  void postPressureEvent(float pressure);
};

#endif
