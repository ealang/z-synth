#include <string.h>
#include "../synth_utils/midi_note_to_freq.h"
#include "./generator_element.h"

using namespace std;

GeneratorElement::GeneratorElement(AudioParams params, function<float(uint32_t, uint32_t)> value)
  : sampleRateHz(params.sampleRateHz),
    channelCount(params.channelCount),
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
    uint32_t channelSize = numSamples * sizeof(float);
    memset(out, 0, channelSize * channelCount);
    return;
  }

  float *dst = out;
  for (uint32_t i = 0; i < numSamples; i++) {
    time = (time + 1) % periodSize;
    float val = value(time, periodSize) * 0.1;
    for (uint32_t c = 0; c < channelCount; ++c)
    {
      *(dst++) = val;
    }
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
