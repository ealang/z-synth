#ifndef LINEAR_SCALE_H
#define LINEAR_SCALE_H

#include <functional>

std::function<float(float)> linearScaleClamped(
  float domainMin, float domainMax,
  float rangeStart, float rangeEnd
);

#endif
