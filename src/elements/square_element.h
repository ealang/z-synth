#ifndef SQUARE_ELEMENT_H
#define SQUARE_ELEMENT_H

#include <cstdint> 
#include "../pipeline/pipeline_element.h"

/* Additive square wave generator.
 * Adds a square wave with a fixed frequency to an input signal.
 */
class SquareElement: public AudioElement<float> {
  const uint32_t sampleRateHz;
  const uint32_t channelCount;
  const float velocity;

  bool off = false;
  uint32_t sampleCount = 0;
  uint32_t offSample = 0;

  float phase = 0;
  const float periodSize;
  const float stepSize;
  const uint32_t attackSampleSize;
  const uint32_t releaseSampleSize;
  const uint32_t decaySampleSize;

public:
  SquareElement(
    uint32_t sampleRateHz,
    uint32_t channelCount,
    float freqHz,
    float velocity
  );

  uint32_t maxInputs() override;
  void generate(
    uint32_t numSamples,
    float* out,
    uint32_t numInputs,
    inputs_t<float> inputs
  ) override;

  bool isExhausted();
  void postOffEvent();
  void postPressureEvent(float pressure);
};

#endif
