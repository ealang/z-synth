#include <cmath>
#include "./generator_functions.h"

#define PI2 (2 * 3.14159265)

float square_function(uint32_t t, uint32_t period) {
  if (t >= (period >> 2)) {
    return 1;
  }
  return -1;
}

float sine_function(uint32_t t, uint32_t period) {
  return sin((float)t / period * PI2);
}
