#include "./generator_element.h"
#include <string.h>

using namespace std;

GeneratorElement::GeneratorElement(uint32_t sampleRateHz, function<float(uint32_t, uint32_t)> value)
  : sampleRateHz(sampleRateHz),
    value(value) {}

uint32_t GeneratorElement::maxInputs() const {
  return 1;
}

uint32_t GeneratorElement::fmPortNumber() const {
  return _fmPortNumber;
}

void GeneratorElement::generate(
  uint32_t numSamples,
  float* out,
  uint32_t numInputs,
  inputs_t<float> inputs
) {
  if (!shouldPlay) {
    memset(out, 0, numSamples * sizeof(float));
    return;
  }

  uint32_t targetPeriodSize = (uint32_t)sampleRateHz / targetFrequency;
  for (uint32_t i = 0; i < numSamples; i++) {
    uint32_t periodSize = numInputs == 0 ?
      targetPeriodSize :
      ((uint32_t)sampleRateHz / (targetFrequency * (1 +  inputs[_fmPortNumber][i] * 0.1f)));

    time = (time + 1) % periodSize;

    float val = value(time, periodSize) * 0.1;
    *(out++) = val;
  }
}

void GeneratorElement::setFrequency(float frequency) {
  targetFrequency = frequency;
}

void GeneratorElement::setEnabled(bool state) {
  shouldPlay = state;
}
