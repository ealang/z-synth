#ifndef AMP_ELEMENT_H
#define AMP_ELEMENT_H

#include "../pipeline/pipeline_element.h"

/**
 * Control amplitude of an input using a multipier.
 */
class AmpElement: public AudioElement<float> {
  float masterAmp;
public:
  AmpElement(float masterAmp);

  uint32_t maxInputs() const override;
  uint32_t inputPortNumber(uint32_t i) const;

  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
