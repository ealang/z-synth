#ifndef DISTORTION_ELEMENT_H
#define DISTORTION_ELEMENT_H

#include "../pipeline/pipeline_element.h"

class DistortionElement: public AudioElement<float> {
  float amount = 0.5;
  void onModChanged(uint8_t value);
public:
  uint32_t maxInputs() override;
  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
