#ifndef FILTER_ELEMENT_H
#define FILTER_ELEMENT_H

#include "../pipeline/pipeline_element.h"

#include <memory>

class RollingAverage;

class FilterElement: public AudioElement<float> {
  std::shared_ptr<RollingAverage> averager;

public:
  FilterElement(uint32_t length);
  uint32_t maxInputs() override;
  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
