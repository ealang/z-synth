#ifndef PRESSURE_DISTORTION_H
#define PRESSURE_DISTORTION_H

#include "../audio_params.h"
#include "../pipeline/pipeline_element.h"

class DistortionElement: public MidiAudioElement<float> {
  AudioParams params;
  float amount = 0;
  Rx::subscription sub;
  void onModChanged(uint8_t value);
public:
  DistortionElement(AudioParams params);
  ~DistortionElement();
  void injectMidi(Rx::observable<const snd_seq_event_t*>) override;
  uint32_t maxInputs() override;
  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
