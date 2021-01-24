#ifndef SQUARE_ELEMENT_H
#define SQUARE_ELEMENT_H

#include "../pipeline/pipeline_element.h"
#include "../synth_utils/monophonic_note_receiver.h"

#include <cstdint> 
#include <functional>

class GeneratorElement: public AudioElement<float>, public MonophonicNoteReceiver {
  const uint32_t sampleRateHz;
  std::function<float(uint32_t, uint32_t)> value;

  bool shouldPlay = false;
  uint32_t time = 0;
  uint32_t periodSize = 0;

  void onVirtualNoteOnEvent(unsigned char note, unsigned char vel) override;
  void onVirtualNoteOffEvent() override;

public:
  GeneratorElement(uint32_t sampleRateHz, std::function<float(uint32_t, uint32_t)> value);

  uint32_t maxInputs() override;

  void replaceValue(std::function<float(uint32_t, uint32_t)> newValue) {
    value = newValue;
  }

  void generate(
    uint32_t numSamples,
    float* out,
    uint32_t numInputs,
    inputs_t<float> inputs
  ) override;
};

#endif
