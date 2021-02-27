#include "./generator_functions.h"

#include <cmath>
#include <cstdlib>
#include <random>
#include <vector>

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
  float value = time * 4;
  if (time > 0.5) {
    value = 4 - value;
  }
  return value - 1;
}

float saw_function(float time) {
  return time * 2 - 1;
}

float reverse_saw_function(float time) {
  return -saw_function(time);
}

float noise_function(float) {
  return (float)rand() / (RAND_MAX / 2) - 1;
}

std::function<float(float)> sampled_noise(uint32_t samplesPerPeriod, int seed) {
  std::default_random_engine generator(seed);
  std::uniform_real_distribution<float> distribution(-1, 1);
  std::vector<float> samples(samplesPerPeriod);
  for (uint32_t i = 0; i < samplesPerPeriod; ++i) {
    samples[i] = distribution(generator);
  }

  return [samples, samplesPerPeriod](float time) {
    uint32_t i;
    if (time <= 0) {
      i = 0;
    } else if (time >= 1) {
      i = samplesPerPeriod - 1;
    } else {
      i = static_cast<uint32_t>(time * samplesPerPeriod);
    }
    return samples[i];
  };
}
