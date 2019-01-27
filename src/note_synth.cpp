#include <algorithm>
#include <math.h>

#include "note_synth.h"

using namespace std;

static const float masterAmp = 0.04f;

static const float attackTimeMs = 10;
static const float releaseTimeMs = 50;
static const float decayTimeMs = 30;
static const float sustainAmp = 0.8f;

NoteSynth::NoteSynth(
    float sampleRateHz,
    float freqHz,
    float velocity
): sampleRateHz(sampleRateHz),
   freqHz(freqHz),
   velocity(velocity),
   attackSampleSize((attackTimeMs / 1000) * sampleRateHz),
   releaseSampleSize((releaseTimeMs / 1000) * sampleRateHz),
   decaySampleSize((decayTimeMs / 1000) * sampleRateHz)
{ }

bool NoteSynth::isExhausted() {
  return off && sampleCount >= offSample + releaseSampleSize;
}

void NoteSynth::generate(uint64_t nSamples, sample_t* buffer) {
  static double max_phase = 2. * M_PI;
  double step = max_phase * freqHz / (double)sampleRateHz;
  unsigned int maxval = (1 << 15) - 1;
  for (uint64_t i = 0; i < nSamples; i++) {
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

    float val = (phase > M_PI ? 1 : -1) * amp * masterAmp;
    sample_t sample = static_cast<sample_t>(val * maxval);
    buffer[i] += sample;
    phase += step;
    sampleCount++;
    if (phase >= max_phase)
      phase -= max_phase;
  }
}

void NoteSynth::postOffEvent() {
  offSample = sampleCount;
  off = true;
}

void NoteSynth::postPressureEvent(float) {
}
