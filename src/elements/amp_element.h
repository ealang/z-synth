#ifndef AMP_ELEMENT_H
#define AMP_ELEMENT_H

#include "../pipeline/pipeline_element.h"

/**
 * Control amplitude of an input using a multipier.
 */
class AmpElement: public AudioElement<float> {
  float amp;
  uint32_t channelCount;
public:
  AmpElement(float amp, uint32_t channelCount);
  uint32_t maxInputs() override;
  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
