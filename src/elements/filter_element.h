#ifndef FILTER_ELEMENT_H
#define FILTER_ELEMENT_H

#include "../pipeline/pipeline_element.h"

#include <memory>

class RollingAverage;

class FilterElement: public AudioElement<float> {
  const uint32_t _inputPortNumber = 0;
  std::shared_ptr<RollingAverage> averager;

public:
  FilterElement(uint32_t length);

  uint32_t maxInputs() const override;
  uint32_t inputPortNumber() const;

  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
