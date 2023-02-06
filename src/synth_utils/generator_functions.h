#ifndef GENERATOR_FUNCTIONS_H
#define GENERATOR_FUNCTIONS_H

#include <cstdint>
#include <functional>

float square_function(float time);
float sine_function(float time);
float triangle_function(float time);
float noise_function(float);
float saw_function(float);
float reverse_saw_function(float);
std::function<float(float)> sampled_noise(uint32_t samplesPerPeriod, int seed = 0);

#endif
