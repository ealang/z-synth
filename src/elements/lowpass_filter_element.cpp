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

static std::vector<float> firFilterWeights(uint32_t sampleRateHz, float cutoffFreq, int n) {
  std::vector<float> weights(n);

  float normalizedFreq = cutoffFreq / (float)sampleRateHz * M_2PI;
  float sum = 0;
  for (auto i = 0; i < n; ++i) {
    float weight = sinc((i - n / 2) * normalizedFreq);
    weights[i] = weight;
    sum += weight;
  }

  for (auto i = 0; i < n; ++i) {
    weights[i] /= sum;
  }

  return weights;
}

LowpassFilterElement::LowpassFilterElement(uint32_t sampleRateHz, float cutoffFreq, int n)
  : _sampleRateHz(sampleRateHz),
    _n(n),
    _averager(
      std::make_shared<WeightedRollingAverage>(
        firFilterWeights(sampleRateHz, cutoffFreq, n)
      )
    )
{}

uint32_t LowpassFilterElement::maxInputs() const {
  return 1;
}


uint32_t LowpassFilterElement::inputPortNumber() const {
  return _inputPortNumber;
}

void LowpassFilterElement::setCutoffFreq(float frequency) {
  _averager->replaceWeights(firFilterWeights(_sampleRateHz, frequency, _n));
}

void LowpassFilterElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  if (numInputs > 0) {
    const float* input = inputs[_inputPortNumber];
    for (uint32_t i = 0; i < numSamples; ++i) {
      *(out++) = _averager->next(*(input++));
    }
  } else {
    memset(out, 0, numSamples * sizeof(float));
  }
}
