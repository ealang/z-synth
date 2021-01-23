#ifndef DISTORTION_ELEMENT_H
#define DISTORTION_ELEMENT_H

#include "../audio_params.h"
#include "../pipeline/pipeline_element.h"

class DistortionElement: public AudioElement<float> {
  AudioParams params;
  float amount = 0.5;
  void onModChanged(uint8_t value);
public:
  DistortionElement(AudioParams params);
  uint32_t maxInputs() override;
  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
