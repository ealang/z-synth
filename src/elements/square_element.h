#ifndef SQUARE_ELEMENT_H
#define SQUARE_ELEMENT_H

#include <cstdint> 
#include "../pipeline/pipeline_element.h"

class SquareElement: public AudioElement<float> {
  const uint32_t sampleRateHz;
  const uint32_t channelCount;
  uint32_t time = 0;
  bool noteIsHeld = false;
  bool sustainIsHeld = false;
  bool shouldPlay = false;
  uint32_t periodSize = 0;

public:
  SquareElement(uint32_t sampleRateHz, uint32_t channelCount);

  void onNoteOnEvent(unsigned char note, unsigned char vel);
  void onNoteOffEvent(unsigned char note);
  void sustainOnEvent();
  void sustainOffEvent();

  uint32_t maxInputs() override;

  void generate(
    uint32_t numSamples,
    float* out,
    uint32_t numInputs,
    inputs_t<float> inputs
  ) override;
};

#endif
