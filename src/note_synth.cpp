#include <stdio.h>
#include <algorithm>
#include <math.h>

#include "note_synth.h"

using namespace std;

static const float attackTimeMs = 10;
static const float releaseTimeMs = 50;
static const float decayTimeMs = 30;
static const float sustainAmp = 0.8f;

NoteSynth::NoteSynth(
    uint32_t sampleRateHz,
    uint32_t channelCount,
    float freqHz,
    float velocity
): sampleRateHz(sampleRateHz),
   channelCount(channelCount),
   freqHz(freqHz),
   velocity(velocity),
   periodSize(sampleRateHz / freqHz),
   attackSampleSize((attackTimeMs / 1000) * sampleRateHz),
   releaseSampleSize((releaseTimeMs / 1000) * sampleRateHz),
   decaySampleSize((decayTimeMs / 1000) * sampleRateHz) {
}

bool NoteSynth::isExhausted() {
  return off && sampleCount >= offSample + releaseSampleSize;
}

void NoteSynth::generate(uint32_t nSamples, float* buffer) {
  uint32_t halfPeriodSize = periodSize / 2;
  for (uint32_t i = 0; i < nSamples; i++) {
    float amp;
    if (sampleCount < attackSampleSize) {
      amp = (float)sampleCount / attackSampleSize;
    } else if (sampleCount < attackSampleSize + decaySampleSize) {
      float p = (sampleCount - attackSampleSize) / (float)decaySampleSize;
      amp = 1 - p * (1 - sustainAmp);
    } else if (off) {
      amp = max(
          0.f,
          (1 - (sampleCount - offSample) / (float)releaseSampleSize) * sustainAmp
      );
    } else {
      amp = sustainAmp;
    }

    float val = (phase > halfPeriodSize ? 1 : -1) * amp * velocity;
    for (uint32_t c = 0; c < channelCount; c++) {
      buffer[i * channelCount + c] += val;
    }

    phase++;
    sampleCount++;
    if (phase >= periodSize) {
      phase -= periodSize;
    }
  }
}

void NoteSynth::postOffEvent() {
  offSample = sampleCount;
  off = true;
}

void NoteSynth::postPressureEvent(float) {
}
