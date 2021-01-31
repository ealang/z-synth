#ifndef DISTORTION_ELEMENT_H
#define DISTORTION_ELEMENT_H

#include "../pipeline/pipeline_element.h"

class DistortionElement: public AudioElement<float> {
  const uint32_t _inputPortNumber = 0;
  const uint32_t _modPortNumber = 1;
  float _defaultAmount, _lowAmount, _highAmount;

public:
  DistortionElement(float defaultAmount = 1, float lowAmount = 0, float highAmount = 1);

  uint32_t maxInputs() const override;
  uint32_t inputPortNumber() const;
  uint32_t modPortNumber() const;

  void setModAmountRange(float lowAmount, float highAmount);
  void setDefaultAmount(float amount);

  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
