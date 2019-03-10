#include <algorithm>
#include <stdio.h>
#include <math.h>

#include "square_element.h"

using namespace std;

SquareElement::SquareElement(
    uint32_t sampleRateHz,
    uint32_t channelCount,
    float freqHz,
    float maxAmp,
    float attackTimeMs,
    float releaseTimeMs,
    float decayTimeMs,
    float _sustainAmp
): sampleRateHz(sampleRateHz),
  channelCount(channelCount),
  maxAmp(maxAmp),
  periodSize(1.0 / freqHz),
  stepSize(1.0 / sampleRateHz),
  attackSampleSize((attackTimeMs / 1000) * sampleRateHz),
  releaseSampleSize((releaseTimeMs / 1000) * sampleRateHz),
  decaySampleSize((decayTimeMs / 1000) * sampleRateHz),
  attackAmp(decaySampleSize == 0 ? _sustainAmp : 1),
  sustainAmp(_sustainAmp),
  releaseAmp(_sustainAmp) {
}

bool SquareElement::isExhausted() {
  return off && sampleCount >= offSample + releaseSampleSize;
}

uint32_t SquareElement::maxInputs() {
  return 0;
}

inline float SquareElement::curAmp() {
  float amp;
  if (off) {
    amp = releaseAmp * max(
        0.f,
        (1 - (sampleCount - offSample) / (float)releaseSampleSize) * sustainAmp
    );
  } else if (sampleCount < attackSampleSize) {
    amp = ((float)sampleCount / attackSampleSize) * attackAmp;
  } else if (sampleCount < attackSampleSize + decaySampleSize) {
    float p = (sampleCount - attackSampleSize) / (float)decaySampleSize;
    amp = 1 - p * (1 - sustainAmp);
  } else {
    amp = sustainAmp;
  }
  return amp;
}

void SquareElement::generate(
  uint32_t numSamples,
  float* out,
  uint32_t,
  inputs_t<float>
) {
  float halfPeriodSize = periodSize / 2;
  for (uint32_t i = 0; i < numSamples; i++) {
    float val = (phase > halfPeriodSize ? 1 : -1) * curAmp() * maxAmp;
    for (uint32_t c = 0; c < channelCount; c++) {
      out[i * channelCount + c] = val;
    }

    phase += stepSize;
    sampleCount++;
    if (phase >= periodSize) {
      phase -= periodSize;
    }
  }
}

void SquareElement::postOffEvent() {
  offSample = sampleCount;
  off = true;
  releaseAmp = curAmp();
}
