#include <algorithm>
#include <math.h>

#include "note_synth.h"

using namespace std;

NoteSynth::NoteSynth(
    float sampleRateHz,
    float freqHz,
    float velocity
): sampleRateHz(sampleRateHz),
   freqHz(freqHz),
   velocity(velocity),
   attackSampleSize((attackTimeMs / 1000) * sampleRateHz),
   releaseSampleSize((releaseTimeMs / 1000) * sampleRateHz)
{ }

bool NoteSynth::isExhausted() {
  return this->off && this->sampleCount >= this->offSample + this->releaseSampleSize;
}

void NoteSynth::generate(uint64_t nSamples, sample_t* buffer) {
  static double max_phase = 2. * M_PI;
  double step = max_phase * this->freqHz / (double)this->sampleRateHz;
  unsigned int maxval = (1 << 15) - 1;
  for (uint64_t i = 0; i < nSamples; i++) {

    float amp;
    if (this->sampleCount < this->attackSampleSize) {
      amp = (float)this->sampleCount / this->attackSampleSize;
    } else if (this->off) {
      amp = max(
          0.f,
          1 - (float)(this->sampleCount - this->offSample) / this->releaseSampleSize
      );
    } else {
      amp = 1;
    }

    float val = sin(this->phase) * amp;
    sample_t sample = static_cast<sample_t>(val * (maxval / 16));
    buffer[i] += sample;
    this->phase += step;
    this->sampleCount++;
    if (this->phase >= max_phase)
      this->phase -= max_phase;
  }
}

void NoteSynth::postOffEvent() {
  this->offSample = this->sampleCount;
  this->off = true;
}

void NoteSynth::postPressureEvent(float) {
}
