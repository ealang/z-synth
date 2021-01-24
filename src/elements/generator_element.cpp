#include <string.h>
#include "../synth_utils/midi_note_to_freq.h"
#include "./generator_element.h"

using namespace std;

GeneratorElement::GeneratorElement(uint32_t sampleRateHz, function<float(uint32_t, uint32_t)> value)
  : sampleRateHz(sampleRateHz),
    value(value) {}

uint32_t GeneratorElement::maxInputs() {
  return 0;
}

void GeneratorElement::generate(
  uint32_t numSamples,
  float* out,
  uint32_t,
  inputs_t<float>
) {
  if (!shouldPlay) {
    memset(out, 0, numSamples * sizeof(float));
    return;
  }

  for (uint32_t i = 0; i < numSamples; i++) {
    time = (time + 1) % periodSize;
    float val = value(time, periodSize) * 0.1;
    *(out++) = val;
  }
}


void GeneratorElement::onVirtualNoteOnEvent(unsigned char note, unsigned char) {
  float freq = midiNoteToFreq(note);
  periodSize = (uint32_t)sampleRateHz / freq;
  shouldPlay = true;
}

void GeneratorElement::onVirtualNoteOffEvent() {
  shouldPlay = false;
}
