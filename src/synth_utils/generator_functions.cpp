#include "./generator_functions.h"

#include <cmath>
#include <cstdlib>

#define PI2 (2 * M_PI)

float square_function(float time) {
  if (time > 0.5) {
    return 1;
  }
  return -1;
}

float sine_function(float time) {
  return sin(time * PI2);
}

float triangle_function(float time) {
  float sign = 1;
  if (time >= 0.5) {
    time -= 0.5;
    sign = -1;
  }

  float value;
  if (time < 0.25) {
    value = time * 4;
  } else {
    value = 1 - (time - 0.25) * 4;
  }

  return value * sign;
}


float saw_function(float time) {
  if (time < 0.5) {
    return time * 2;
  }
  return (time - 0.5) * 2 - 1;
}

float noise_function(float) {
  return (float)rand() / (RAND_MAX / 2) - 1;
}

