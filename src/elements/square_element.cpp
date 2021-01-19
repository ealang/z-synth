#include <string.h>
#include "../synth_utils/midi_note_to_freq.h"
#include "./square_element.h"

using namespace std;

SquareElement::SquareElement(uint32_t sampleRateHz, uint32_t channelCount)
  : sampleRateHz(sampleRateHz),
    channelCount(channelCount) {}

uint32_t SquareElement::maxInputs() {
  return 0;
}

void SquareElement::generate(
  uint32_t numSamples,
  float* out,
  uint32_t,
  inputs_t<float>
) {
  if (!shouldPlay)
  {
    uint32_t channelSize = numSamples * sizeof(float);
    memset(out, 0, channelSize * channelCount);
    return;
  }

  // generate first channel
  uint32_t halfPeriodSize = periodSize / 2;
  float *dst = out;
  for (uint32_t i = 0; i < numSamples; i++) {
    time = (time + 1) % periodSize;
    float val = time >= halfPeriodSize ? .1 : -.1;
    for (uint32_t c = 0; c < channelCount; ++c)
    {
      *(dst++) = val;
    }
  }
}

void SquareElement::noteOnEvent(unsigned char note, unsigned char ) {
  float freq = midiNoteToFreq(note);
  periodSize = (uint32_t)sampleRateHz / freq;
  noteIsHeld = true;
  shouldPlay = true;
}

void SquareElement::noteOffEvent(unsigned char ) {
  noteIsHeld = false;
  if (!sustainIsHeld)
  {
    shouldPlay = false;
  }
}

void SquareElement::sustainOnEvent() {
  sustainIsHeld = true;
}

void SquareElement::sustainOffEvent() {
  sustainIsHeld = false;
  if (!noteIsHeld)
  {
    shouldPlay = false;
  }
}
