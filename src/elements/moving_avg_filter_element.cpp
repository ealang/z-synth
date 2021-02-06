#include "./moving_avg_filter_element.h"

#include <cstring>
#include <vector>

using namespace std;

static const float pullDown = 0.999;

class RollingAverage {
  uint32_t length;

  std::vector <float> buffer;
  uint32_t bufferPos = 0;

  float sum = 0;

public:
  RollingAverage(uint32_t length):
    length(length),
    buffer(length) {}

  float next(float val) {
    if (length == 0) {
      return val;
    }
    sum = (sum + val - buffer[bufferPos]) * pullDown;
    buffer[bufferPos] = val;
    bufferPos = (bufferPos + 1) % length;
    return sum / length;
  }
};

MovingAvgFilterElement::MovingAvgFilterElement(uint32_t length)
  : averager(make_shared<RollingAverage>(length)) {
}

uint32_t MovingAvgFilterElement::maxInputs() const {
  return 1;
}


uint32_t MovingAvgFilterElement::inputPortNumber() const {
  return _inputPortNumber;
}

void MovingAvgFilterElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  if (numInputs > 0) {
    const float* input = inputs[_inputPortNumber];
    for (uint32_t i = 0; i < numSamples; i++) {
      *(out++) = averager->next(*(input++));
    }
  } else {
    memset(out, 0, numSamples * sizeof(float));
  }
}
