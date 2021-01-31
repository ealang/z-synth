#include "./generator_element.h"
#include <cmath>
#include <cstring>

using namespace std;

GeneratorElement::GeneratorElement(uint32_t sampleRateHz, function<float(float)> value)
  : sampleRateHz(sampleRateHz),
    _value(value) {}

uint32_t GeneratorElement::maxInputs() const {
  return 2;
}

uint32_t GeneratorElement::fmPortNumber() const {
  return _fmPortNumber;
}

uint32_t GeneratorElement::amPortNumber() const {
  return _amPortNumber;
}

void GeneratorElement::generate(
  uint32_t numSamples,
  float* out,
  uint32_t numInputs,
  inputs_t<float> inputs
) {
  bool usingFm =
    numInputs > _fmPortNumber &&
    inputs[_fmPortNumber] &&
    _fmRange > 0;
  bool usingAm =
    numInputs > _amPortNumber &&
    inputs[_amPortNumber];

  if (!isEnabled || (!usingAm && _amplitude == 0) || (!usingFm && _frequency == 0)) {
    memset(out, 0, numSamples * sizeof(float));
    return;
  }

  const float *fmIn = usingFm ? inputs[_fmPortNumber] : nullptr;
  const float *amIn = usingAm ? inputs[_amPortNumber] : nullptr;

  float timeStep = _frequency / sampleRateHz;
  float instFreq = _frequency;
  float instAmp = _amplitude;

  for (uint32_t i = 0; i < numSamples; i++) {
    if (usingFm) {
      instFreq = _frequency + _fmRange * *(fmIn++);
      timeStep = instFreq / sampleRateHz;
    }
    if (usingAm) {
      instAmp = _amplitude * *(amIn++);
    }

    time = fmod(time + timeStep, 1);
    *(out++) = _value(time) * instAmp;
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
