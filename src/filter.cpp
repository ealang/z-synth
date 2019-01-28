#include "filter.h"

static const float pullDown = 0.999;

Filter::Filter(uint32_t length):
  length(length),
  buffer(length) {
}

float Filter::next(float val) {
  sum = (sum + val - buffer[bufferPos]) * pullDown;
  buffer[bufferPos] = val;
  bufferPos = (bufferPos + 1) % length;
  return sum / length;
}
