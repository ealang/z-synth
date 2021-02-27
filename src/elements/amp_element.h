#ifndef AMP_ELEMENT_H
#define AMP_ELEMENT_H

#include "../pipeline/pipeline_element.h"

class AmpElement: public AudioElement<float> {
  float _maxOutputValue;
  float _inputSaturation;
  float _inputAmplification;
  float _domainScale;

public:
  AmpElement();

  uint32_t maxInputs() const override;
  uint32_t inputPortNumber(uint32_t i) const;

  void setInputSaturationValue(float value);
  void setInputAmplificationValue(float value);
  void setMaxOutputValue(float maxValue);

  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
