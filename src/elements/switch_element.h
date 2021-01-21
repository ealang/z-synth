#ifndef SWITCH_ELEMENT_H
#define SWITCH_ELEMENT_H

#include "../pipeline/pipeline_element.h"

#include <memory>

/**
 * Allow dynamic switching between AudioElements.
 */
class SwitchElement: public AudioElement<float> {
  uint32_t channelCount;
  std::shared_ptr<AudioElement<float>> core;
public:
  SwitchElement(uint32_t channelCount);
  uint32_t maxInputs() override;
  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;

  void setCore(std::shared_ptr<AudioElement<float>> new_core);
};

#endif
