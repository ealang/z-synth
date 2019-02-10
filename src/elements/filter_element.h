#ifndef FILTER_ELEMENT_H
#define FILTER_ELEMENT_H

#include <memory>
#include <vector>

#include "../pipeline/pipeline_element.h"

class RollingAverage;

class FilterElement: public AudioElement<float> {
  const uint32_t channelCount;
  std::vector<std::shared_ptr<RollingAverage>> averagers;

public:
  FilterElement(float timeSec, uint32_t sampleRateHz, uint32_t channelCount);
  uint32_t nInputs() const;
  void generate(uint32_t nSamples, float* out, const float** in);
};

#endif