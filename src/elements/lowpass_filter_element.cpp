#include "./lowpass_filter_element.h"
#include "../synth_utils/weighted_rolling_average.h"

#include <cstring>
#include <cmath>

constexpr float M_2PI = M_PI * 2;
constexpr float filterDebounceHz = 1;

static std::vector<float> firFilterWeights(uint32_t sampleRateHz, float cutoffFreq, uint32_t n);

class FilterCache {
  static constexpr float startHz = 0;
  static constexpr float endHz = 20000;
  static constexpr float stepHz = 1;
  static uint32_t _sampleRateHz;
  static uint32_t _n;
  static std::vector<std::vector<float>> cache;

public:
  static void init(uint32_t sampleRateHz, uint32_t n) {
    _sampleRateHz = sampleRateHz;
    _n = n;

    uint32_t numEntries = (endHz - startHz) / stepHz + 1;
    float cutoff = startHz;
    for (uint32_t i = 0; i < numEntries; ++i) {
      cache.emplace_back(firFilterWeights(sampleRateHz, cutoff, _n));
      cutoff += stepHz;
    }
  }

  static const std::vector<float>& lookup(uint32_t sampleRateHz, float frequency, uint32_t n) {
    if (cache.size() == 0) {
      init(sampleRateHz, n);
    }
    if (n != _n || sampleRateHz != _sampleRateHz) {
      throw std::runtime_error("Filter params changed");
    }

    uint32_t i;
    if (frequency <= startHz) {
      i = 0;
    } else if (frequency >= endHz) {
      i = cache.size() - 1;
    } else {
      i = (frequency - startHz) / stepHz;
    }
    return cache[i];
  }
};

uint32_t FilterCache::_sampleRateHz;
uint32_t FilterCache::_n;
std::vector<std::vector<float>> FilterCache::cache;

static float sinc(float x) {
  if (x == 0) {
    return 1;
  }
  return sin(x) / x;
}

static std::vector<float> firFilterWeights(uint32_t sampleRateHz, float cutoffFreq, uint32_t n) {
  std::vector<float> weights(n);
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

  return weights;
}

LowpassFilterElement::LowpassFilterElement(uint32_t sampleRateHz, float cutoffFreq, uint32_t n)
  : _sampleRateHz(sampleRateHz),
    _n(n),
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
  _averager->setWeights(
    FilterCache::lookup(_sampleRateHz, cutoffFreq, _n)
  );
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

      if (fabs(cutoffFreq - _curCutoffFreq) > filterDebounceHz) {
        replaceWeights(cutoffFreq);
      }

      *(out++) = _averager->next(*(sigIn++));
    }
  } else {
    memset(out, 0, numSamples * sizeof(float));
  }
}
