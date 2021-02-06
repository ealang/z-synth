#include "./lowpass_filter_element.h"
#include "../synth_utils/weighted_rolling_average.h"

#include <cstring>
#include <cmath>

#define M_2PI (M_PI * 2)

static float sinc(float x) {
  if (x == 0) {
    return 1;
  }
  return sin(x) / x;
}

static void firFilterWeights(std::vector<float>& weights, uint32_t sampleRateHz, float cutoffFreq) {
  const uint32_t n = weights.size();
  float normalizedFreq = cutoffFreq / (float)sampleRateHz * M_2PI;

  float sum = 0;

  for (uint32_t i = 0; i < n; ++i) {
    float weight = sinc((i - n / 2) * normalizedFreq);
    weights[i] = weight;
    sum += weight;
  }

  for (uint32_t i = 0; i < n; ++i) {
    weights[i] /= sum;
  }
}

LowpassFilterElement::LowpassFilterElement(uint32_t sampleRateHz, float cutoffFreq, int n)
  : _sampleRateHz(sampleRateHz),
    _baseCutoffFreq(cutoffFreq),
    _curCutoffFreq(cutoffFreq),
    _averager(std::make_shared<WeightedRollingAverage>(n))
{
  replaceWeights(cutoffFreq);
}

uint32_t LowpassFilterElement::maxInputs() const {
  return 2;
}


uint32_t LowpassFilterElement::inputPortNumber() const {
  return _inputPortNumber;
}

uint32_t LowpassFilterElement::modPortNumber() const {
  return _modPortNumber;
}

void LowpassFilterElement::replaceWeights(float cutoffFreq) {
  _curCutoffFreq = cutoffFreq;
  firFilterWeights(_averager->weights(), _sampleRateHz, cutoffFreq);
}

void LowpassFilterElement::setCutoffFreq(float frequency) {
  _baseCutoffFreq = frequency;
}

void LowpassFilterElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  bool inputConnected = numInputs > _inputPortNumber && inputs[_inputPortNumber];

  float cutoffFreq = _baseCutoffFreq;
  if (inputConnected) {
    const float* sigIn = inputs[_inputPortNumber];

    bool modConnected = numInputs > _modPortNumber && inputs[_modPortNumber];
    const float* modIn = modConnected ? inputs[_modPortNumber] : nullptr;

    for (uint32_t i = 0; i < numSamples; ++i) {
      if (modIn) {
        cutoffFreq = *(modIn++) * _baseCutoffFreq;
      }

      if (cutoffFreq != _curCutoffFreq) {
        replaceWeights(cutoffFreq);
      }

      *(out++) = _averager->next(*(sigIn++));
    }
  } else {
    memset(out, 0, numSamples * sizeof(float));
  }
}
