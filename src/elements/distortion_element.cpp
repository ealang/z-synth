#include "./distortion_element.h"

#include <cstring>
#include <cmath>

DistortionElement::DistortionElement(float amount)
  : amount(amount) {}

uint32_t DistortionElement::maxInputs() const {
  return 1;
}

uint32_t DistortionElement::inputPortNumber() const {
  return _inputPortNumber;
}

void DistortionElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  if (numInputs > 0 && amount > 0) {
    const float* in = inputs[_inputPortNumber];
    for (uint32_t i = 0; i < numSamples; i++) {
      *(out++) = tanh(*(in++) * (0.5 + amount * 4));
    }
  } else {
    memset(out, 0, numSamples * sizeof(float));
  }
}
