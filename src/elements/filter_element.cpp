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
    sum = (sum + val - buffer[bufferPos]) * pullDown;
    buffer[bufferPos] = val;
    bufferPos = (bufferPos + 1) % length;
    return sum / length;
  }
};

FilterElement::FilterElement(float timeSec, uint32_t sampleRateHz, uint32_t channelCount)
  : channelCount(channelCount) {
    uint32_t length = max((uint32_t)1, static_cast<uint32_t>(timeSec * sampleRateHz));
    for (uint32_t c = 0; c < channelCount; ++c) {
      averagers.push_back(make_shared<RollingAverage>(length));
    }
}

uint32_t FilterElement::nInputs() const {
  return 1;
}

void FilterElement::generate(uint32_t nSamples, float* out, const float** inputs) {
  const float* input = inputs[0];
  for (uint32_t i = 0; i < nSamples; i++) {
    for (uint32_t c = 0; c < channelCount; c++) {
      out[i * channelCount + c] = averagers[c]->next(input[i]);
    }
  }
}