#ifndef LOWPASS_FILTER_ELEMENT_H
#define LOWPASS_FILTER_ELEMENT_H

#include "../pipeline/pipeline_element.h"

#include <memory>

class WeightedRollingAverage;

class LowpassFilterElement: public AudioElement<float> {
  static constexpr uint32_t _inputPortNumber = 0;
  static constexpr uint32_t _modPortNumber = 1;
  const uint32_t _sampleRateHz;
  const uint32_t _n;
  float _baseCutoffFreq;
  float _curCutoffFreq;
  std::shared_ptr<WeightedRollingAverage> _averager;

  void replaceWeights(float cutoffFreq);

public:
  LowpassFilterElement(uint32_t sampleRateHz, float cutoffFreq, uint32_t n);

  uint32_t maxInputs() const override;
  uint32_t inputPortNumber() const;
  uint32_t modPortNumber() const;

  void setCutoffFreq(float frequency);

  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
