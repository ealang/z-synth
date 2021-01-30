#include "./generator_element.h"
#include <cmath>
#include <cstring>

using namespace std;

GeneratorElement::GeneratorElement(uint32_t sampleRateHz, function<float(float)> value)
  : sampleRateHz(sampleRateHz),
    _value(value) {}

uint32_t GeneratorElement::maxInputs() const {
  return 1;
}

uint32_t GeneratorElement::fmPortNumber() const {
  return _fmPortNumber;
}

void GeneratorElement::generateFrequencyMod(uint32_t numSamples, float *out, const float *fmIn) {
  for (uint32_t i = 0; i < numSamples; i++) {
    float instFreq = _frequency + _fmRange * *(fmIn++);
    float timeStep = instFreq / sampleRateHz;
    time = fmod(time + timeStep, 1);

    *(out++) = _value(time) * _amplitude;
  }
}

void GeneratorElement::generateFixedFrequency(uint32_t numSamples, float *out) {
  float timeStep = _frequency / sampleRateHz;

  for (uint32_t i = 0; i < numSamples; i++) {
    time = fmod(time + timeStep, 1);
    *(out++) = _value(time) * _amplitude;
  }
}

void GeneratorElement::generate(
  uint32_t numSamples,
  float* out,
  uint32_t numInputs,
  inputs_t<float> inputs
) {
  bool fixedFrequency = numInputs == 0 || _fmRange == 0;
  if (!isEnabled || _amplitude == 0 || (fixedFrequency && _frequency == 0)) {
    memset(out, 0, numSamples * sizeof(float));
    return;
  }

  if (fixedFrequency) {
    generateFixedFrequency(numSamples, out);
  } else {
    generateFrequencyMod(numSamples, out, inputs[0]);
  }
}

void GeneratorElement::setValue(std::function<float(float)> value) {
  _value = value;
}

void GeneratorElement::setFrequency(float frequency) {
  _frequency = frequency;
}

void GeneratorElement::setEnabled(bool state) {
  isEnabled = state;
}

void GeneratorElement::setAmplitude(float amplitude) {
  _amplitude = amplitude;
}

void GeneratorElement::setFMLinearRange(float frequency) {
  _fmRange = frequency;
}
