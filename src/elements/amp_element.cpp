#include "./amp_element.h"

#include <cstring>
#include <cmath>

static constexpr float tanhDomainScale = 4;

static inline float valueAt(float x, float amount) {
  return tanh(x * (1 + amount * tanhDomainScale));
}

AmpElement::AmpElement(float amp):
  _amp(amp) {
}

uint32_t AmpElement::maxInputs() const {
  return -1;
}

uint32_t AmpElement::inputPortNumber(uint32_t i) const {
  return i;
}

void AmpElement::setAmp(float amp) {
  _amp = amp;
}

void AmpElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  memset(out, 0, sizeof(float) * numSamples);
  for (uint32_t i = 0; i < numInputs; ++i) {
    const float *inBuffer = inputs[i];
    if (inBuffer != nullptr) {
      float *outBuffer = out;
      for (uint32_t s = 0; s < numSamples; ++s) {
        *(outBuffer++) += *(inBuffer++) * _amp;
      }
    }
  }

  float *outBuffer = out;
  for (uint32_t i = 0; i < numSamples; ++i) {
    *outBuffer = valueAt(*outBuffer, 1);
    ++outBuffer;
  }
};
