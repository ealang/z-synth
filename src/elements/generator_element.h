#ifndef SQUARE_ELEMENT_H
#define SQUARE_ELEMENT_H

#include "../pipeline/pipeline_element.h"

#include <cstdint> 
#include <functional>

class GeneratorElement: public AudioElement<float> {
  const uint32_t _fmPortNumber = 0;
  const uint32_t _amPortNumber = 1;

  const uint32_t sampleRateHz;
  std::function<float(float)> _value;
  float _frequency = 0;
  float _amplitude = 1;
  float _fmRange = 0;

  bool isEnabled = false;
  float time = 0;

public:
  GeneratorElement(uint32_t sampleRateHz, std::function<float(float)> value);

  uint32_t maxInputs() const override;
  // Port for frequency modulation
  uint32_t fmPortNumber() const;
  // Port for amplitude modulation
  uint32_t amPortNumber() const;

  void setValue(std::function<float(float)> value);
  void setFrequency(float frequency);
  void setEnabled(bool state);
  void setAmplitude(float amplitude);
  void setFMLinearRange(float frequency);

  void generate(
    uint32_t numSamples,
    float* out,
    uint32_t numInputs,
    inputs_t<float> inputs
  ) override;
};

#endif
