#include "./mixer_element.h"

#include <stdexcept>
#include <cstring>

MixerElement::MixerElement(uint32_t n)
  : _weights(n, 1. / n) {}

MixerElement::MixerElement(uint32_t n, float weight)
  : _weights(n, weight) {}

MixerElement::MixerElement(std::vector<float> weights)
  : _weights(weights) {}

uint32_t MixerElement::maxInputs() const {
  return _weights.size();
}

uint32_t MixerElement::inputPortNumber(uint32_t i) const {
  return i;
}

void MixerElement::setWeight(uint32_t i, float weight) {
  _weights[i] = weight;
}

void MixerElement::setWeights(std::vector<float> weights) {
  if (weights.size() != _weights.size()) {
    throw std::runtime_error("Changed num inputs");
  }
  _weights = weights;
}

void MixerElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  memset(out, 0, sizeof(float) * numSamples);

  for (uint32_t i = 0; i < numInputs; ++i) {
    const float *inBuffer = inputs[i];
    if (inBuffer != nullptr) {
      const float amp = _weights[i];
      float *outBuffer = out;
      for (uint32_t s = 0; s < numSamples; ++s) {
        *(outBuffer++) += *(inBuffer++) * amp;
      }
    }
  }
};
