#include "./amp_element.h"

#include <cstring>
#include <cmath>

static constexpr float defaultMaxOutput = 1;
static constexpr float defaultInputSaturation = 1;
static constexpr float defaultInputAmplification = 1;

static inline float calcDomainScale(float amp, float sat) {
  return amp / sat * 2;
}

AmpElement::AmpElement():
  _maxOutputValue(defaultMaxOutput),
  _inputSaturation(defaultInputSaturation),
  _inputAmplification(defaultInputAmplification),
  _domainScale(calcDomainScale(defaultInputAmplification, defaultInputSaturation)) {
}

uint32_t AmpElement::maxInputs() const {
  return -1;
}

uint32_t AmpElement::inputPortNumber(uint32_t i) const {
  return i;
}

void AmpElement::setInputSaturationValue(float value) {
  _inputSaturation = value;
  _domainScale = calcDomainScale(_inputAmplification, _inputSaturation);
}

void AmpElement::setInputAmplificationValue(float value) {
  _inputAmplification = value;
  _domainScale = calcDomainScale(_inputAmplification, _inputSaturation);
}

void AmpElement::setMaxOutputValue(float maxValue) {
  _maxOutputValue = maxValue;
}

void AmpElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  memset(out, 0, sizeof(float) * numSamples);

  for (uint32_t i = 0; i < numInputs; ++i) {
    const float *inBuffer = inputs[i];
    if (inBuffer != nullptr) {
      float *outBuffer = out;
      for (uint32_t s = 0; s < numSamples; ++s) {
        *(outBuffer++) += *(inBuffer++);
      }
    }
  }

  float *outBuffer = out;
  for (uint32_t i = 0; i < numSamples; ++i) {
    *outBuffer = tanh(*outBuffer * _domainScale) * _maxOutputValue;
    ++outBuffer;
  }
};
