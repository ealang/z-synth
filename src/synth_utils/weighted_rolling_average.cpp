#include "./weighted_rolling_average.h"

#include <stdexcept>

WeightedRollingAverage::WeightedRollingAverage(int n)
  : _weights(n, 1. / n),
    _buffer(n, 0)
{}

WeightedRollingAverage::WeightedRollingAverage(std::vector<float> weights)
  : _weights(weights),
    _buffer(weights.size(), 0)
{}

void WeightedRollingAverage::setWeights(std::vector<float> weights) {
  if (weights.size() != _weights.size()) {
    throw std::runtime_error("Weights size changed");
  }
  _weights = weights;
}

float WeightedRollingAverage::next(float value) {
  const uint32_t n = _weights.size();
  if (n == 0) {
    return value;
  }

  float result = 0;
  _buffer[_bufferPos] = value;
  _bufferPos = (_bufferPos + 1) % n;

  for (uint32_t i = 0; i < n; ++i) {
    result += _buffer[(_bufferPos + i) % n] * _weights[i];
  }

  return result;
}
