#include "./distortion_element.h"

#include <cstring>
#include <cmath>

DistortionElement::DistortionElement(float defaultAmount, float lowAmount, float highAmount)
  : _defaultAmount(defaultAmount),
    _lowAmount(lowAmount),
    _highAmount(highAmount) {}

uint32_t DistortionElement::maxInputs() const {
  return 2;
}

uint32_t DistortionElement::inputPortNumber() const {
  return _inputPortNumber;
}

uint32_t DistortionElement::modPortNumber() const {
  return _modPortNumber;
}

void DistortionElement::setModAmountRange(float lowAmount, float highAmount) {
  _lowAmount = lowAmount;
  _highAmount = highAmount;
}

void DistortionElement::setDefaultAmount(float amount) {
  _defaultAmount = amount;
}

void DistortionElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  const float* sourceIn = numInputs > _inputPortNumber ? inputs[_inputPortNumber] : nullptr;
  if (sourceIn) {
    const float* modIn = numInputs > _modPortNumber ? inputs[_modPortNumber] : nullptr;
    float amount = _defaultAmount;
    for (uint32_t i = 0; i < numSamples; i++) {
      if (modIn) {
        amount = *(modIn++);
      }
      *(out++) = tanh(*(sourceIn++) * (0.5 + amount * 4));
    }
  } else {
    memset(out, 0, numSamples * sizeof(float));
  }
}
