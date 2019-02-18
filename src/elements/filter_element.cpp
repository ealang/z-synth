#include "./filter_element.h"

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

FilterElement::FilterElement(uint32_t length, uint32_t channelCount)
  : channelCount(channelCount) {
    for (uint32_t c = 0; c < channelCount; ++c) {
      averagers.push_back(make_shared<RollingAverage>(length));
    }
}

uint32_t FilterElement::maxInputs() {
  return 1;
}

void FilterElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  if (numInputs > 0) {
    const float* input = inputs[0];
    for (uint32_t i = 0; i < numSamples * channelCount; i++) {
      out[i] = averagers[i % channelCount]->next(input[i]);
    }
  }
}
