#ifndef AMP_ELEMENT_H
#define AMP_ELEMENT_H

#include "../pipeline/pipeline_element.h"

/**
 * Control amplitude of an input using a multipier.
 */
class AmpElement: public AudioElement<float>, public MidiListener {
  float masterAmp;
  float channelAmp = 1;
  uint32_t channelCount;
  Rx::subscription sub;
public:
  AmpElement(float masterAmp, uint32_t channelCount);
  ~AmpElement();
  void injectMidi(Rx::observable<const snd_seq_event_t*>) override;
  uint32_t maxInputs() override;
  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
