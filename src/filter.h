#ifndef FILTER_H
#define FILTER_H

#include <vector>
#include <cstdint> 

class Filter {
  uint32_t length;

  std::vector <float> buffer;
  uint32_t bufferPos = 0;

  float sum = 0;

public:
  Filter(uint32_t length);
  float next(float val);
};

#endif
