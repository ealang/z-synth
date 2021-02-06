#ifndef WEIGHTED_ROLLING_AVERAGE_H
#define WEIGHTED_ROLLING_AVERAGE_H

#include <cstdint>
#include <vector>

class WeightedRollingAverage {
  std::vector <float> _weights;
  std::vector <float> _buffer;
  uint32_t _bufferPos = 0;

public:
  WeightedRollingAverage(int n);
  WeightedRollingAverage(std::vector<float> weights);

  std::vector<float>& weights();
  float next(float value);
};

#endif
