#include <algorithm>
#include <cassert>
#include <stdio.h>
#include <math.h>

#include "square_element.h"

using namespace std;

static const float attackTimeMs = 10;
static const float releaseTimeMs = 50;
static const float decayTimeMs = 30;
static const float sustainAmp = 0.5f;

SquareElement::SquareElement(
    uint32_t sampleRateHz,
    uint32_t channelCount,
    float freqHz,
    float velocity
): sampleRateHz(sampleRateHz),
  channelCount(channelCount),
  velocity(velocity),
  periodSize(1.0 / freqHz),
  stepSize(1.0 / sampleRateHz),
  attackSampleSize((attackTimeMs / 1000) * sampleRateHz),
  releaseSampleSize((releaseTimeMs / 1000) * sampleRateHz),
  decaySampleSize((decayTimeMs / 1000) * sampleRateHz) {
}

bool SquareElement::isExhausted() {
  return off && sampleCount >= offSample + releaseSampleSize;
}

uint32_t SquareElement::maxInputs() {
  return 1;
}

void SquareElement::generate(
  uint32_t numSamples,
  float* out,
  uint32_t numInputs,
  inputs_t<float> inputs
) {
  assert(numInputs == 1);
  const float* input = inputs[0];
  float halfPeriodSize = periodSize / 2;
  for (uint32_t i = 0; i < numSamples; i++) {
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
      uint32_t s = i * channelCount + c;
      out[s] = input[s] + val;
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
}

void SquareElement::postPressureEvent(float) {
}
