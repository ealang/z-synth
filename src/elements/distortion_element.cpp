#include "./distortion_element.h"

#include <cstring>
#include <cmath>

DistortionElement::DistortionElement(AudioParams params):
  params(params) {}

uint32_t DistortionElement::maxInputs() {
  return 1;
}

void DistortionElement::onModChanged(uint8_t value) {
  amount = value/127.;
}

void DistortionElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  uint32_t total = numSamples * params.channelCount;
  if (numInputs > 0) {
    const float* in = inputs[0];
    if (amount > 0) {
      for (uint32_t i = 0; i < total; i++) {
        out[i] = tanh(in[i] * (0.5 + amount * 4));
      }
    } else {
      memcpy(out, in, total * sizeof(float));
    }
  } else {
    memset(out, 0, total * sizeof(float));
  }
}
