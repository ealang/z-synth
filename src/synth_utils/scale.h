#ifndef SCALE_H
#define SCALE_H

#include <functional>

std::function<float(float)> linearScaleClamped(
  float domainMin, float domainMax,
  float rangeStart, float rangeEnd
);

std::function<float(float)> powerScaleClamped(
  float n,
  float domainStart, float domainEnd,
  float rangeMin, float rangeMax
);

#endif
