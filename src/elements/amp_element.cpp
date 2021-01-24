#include "./amp_element.h"

#include <cstring>

AmpElement::AmpElement(float masterAmp):
  masterAmp(masterAmp) {
}

uint32_t AmpElement::maxInputs() {
  return -1;
}

void AmpElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  memset(out, 0, sizeof(float) * numSamples);
  for (uint32_t i = 0; i < numInputs; i++) {
    const float *inBuffer = inputs[i];
    float *outBuffer = out;
    for (uint32_t s = 0; s < numSamples; s++) {
      *(outBuffer++) += *(inBuffer++) * masterAmp;
    }
  }
};
