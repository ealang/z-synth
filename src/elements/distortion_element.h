#ifndef DISTORTION_ELEMENT_H
#define DISTORTION_ELEMENT_H

#include "../pipeline/pipeline_element.h"

class DistortionElement: public AudioElement<float> {
  const uint32_t _inputPortNumber = 0;
  float amount;

public:
  DistortionElement(float amount);

  uint32_t maxInputs() const override;
  uint32_t inputPortNumber() const;

  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
