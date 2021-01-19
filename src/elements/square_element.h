#ifndef SQUARE_ELEMENT_H
#define SQUARE_ELEMENT_H

#include <cstdint> 
#include "../pipeline/pipeline_element.h"
#include "../synth_utils/note_listener.h"

class SquareElement: public AudioElement<float>, public NoteListener {
  const uint32_t sampleRateHz;
  const uint32_t channelCount;
  uint32_t time = 0;
  bool noteIsHeld = false;
  bool sustainIsHeld = false;
  bool shouldPlay = false;
  uint32_t periodSize = 0;

  void noteOnEvent(unsigned char note, unsigned char vel) override;
  void noteOffEvent(unsigned char note) override;
  void sustainOnEvent() override;
  void sustainOffEvent() override;

public:
  SquareElement(uint32_t sampleRateHz, uint32_t channelCount);

  uint32_t maxInputs() override;

  void generate(
    uint32_t numSamples,
    float* out,
    uint32_t numInputs,
    inputs_t<float> inputs
  ) override;
};

#endif
