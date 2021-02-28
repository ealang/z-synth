#ifndef MATH_MULT_ADD_ELEMENT_H
#define MATH_MULT_ADD_ELEMENT_H

#include "../pipeline/pipeline_element.h"

// Multiply and add operation to the input.
class MathMultAddElement: public AudioElement<float> {
  float _multVal;
  float _addVal;

public:
  MathMultAddElement(float multVal, float addVal);

  uint32_t maxInputs() const override;
  uint32_t inputPortNumber() const;

  void setMultValue(float value);
  void setAddValue(float value);

  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
