#include "./amp_element.h"

AmpElement::AmpElement(float amp, uint32_t channelCount):
  amp(amp), channelCount(channelCount) {
}

uint32_t AmpElement::nInputs() const {
  return 1;
}

void AmpElement::generate(uint32_t nSamples, float* out, const float** ins) {
  const float *in = ins[0];
  for (uint32_t i = 0; i < nSamples * channelCount; i++) {
    out[i] = in[i] * amp;
  }
};
