#include "./scale.h"

#include <cmath>

std::function<float(float)> linearScaleClamped(float domainMin, float domainMax, float rangeStart, float rangeEnd) {
  const float domainSize = domainMax - domainMin;
  const float rangeDelta = rangeEnd - rangeStart;

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

std::function<float(float)> powerScaleClamped(float n, float domainStart, float domainEnd, float rangeMin, float rangeMax) {
  const float rangeSize = rangeMax - rangeMin;
  const float domainDelta = domainEnd - domainStart;
  const float c = pow(0.5, n);

  return [=](float domainInput) {
    float x = (domainInput - domainStart) / domainDelta;
    if (x <= 0) {
      x = 0;
    }
    if (x >= 1) {
      x = 1;
    }

    return (1 / pow(2 - x, n) - c) * (1 / (1 - c)) * rangeSize + rangeMin;
  };
}
