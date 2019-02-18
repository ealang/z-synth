#include "./linear_scale.h"

std::function<float(float)> linearScaleClamped(float domainMin, float domainMax, float rangeStart, float rangeEnd) {
  float domainSize = domainMax - domainMin;
  float rangeDelta = rangeEnd - rangeStart;
  return [=](float domainInput) {
    float x = (domainInput - domainMin) / domainSize;
    if (x <= 0) {
      return rangeStart;
    }
    if (x >= 1) {
      return rangeEnd;
    }
    return x * rangeDelta + rangeStart;
  };
}
