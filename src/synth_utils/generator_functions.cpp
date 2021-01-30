#include <cmath>
#include "./generator_functions.h"

#define PI2 (2 * 3.14159265)

float square_function(float time) {
  if (time > 0.5) {
    return 1;
  }
  return -1;
}

float sine_function(float time) {
  return sin(time * PI2);
}
