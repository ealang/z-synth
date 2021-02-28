#include "./math_mult_add_element.h"

#include <cstring>

MathMultAddElement::MathMultAddElement(float multVal, float addVal)
  : _multVal(multVal), _addVal(addVal) {
}

uint32_t MathMultAddElement::maxInputs() const {
  return 1;
}

uint32_t MathMultAddElement::inputPortNumber() const {
  return 0;
}

void MathMultAddElement::setMultValue(float value) {
  _multVal = value;
}

void MathMultAddElement::setAddValue(float value) {
  _addVal = value;
}

void MathMultAddElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  const float *in = numInputs > 0 ? inputs[0] : nullptr;

  if (in == nullptr) {
    memset(out, 0, sizeof(float) * numSamples);
    return;
  }

  for (uint32_t i = 0; i < numSamples; ++i) {
    *(out++) = (*(in++) * _multVal) + _addVal;
  }
};
