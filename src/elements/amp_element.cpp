#include <cstring>
#include "./amp_element.h"
#include "../synth_utils/midi_filters.h"

AmpElement::AmpElement(float masterAmp, uint32_t channelCount):
  masterAmp(masterAmp), channelCount(channelCount) {
}

AmpElement::~AmpElement() {
  sub.unsubscribe();
}

void AmpElement::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  sub = midi
    | Rx::filter(controlFilter(MIDI_PARAM_CHANNEL_VOLUME))
    | Rx::map(controlMap)
    | Rx::subscribe<int>([this](int value) {
        channelAmp = value / 127.;
      });
}

uint32_t AmpElement::maxInputs() {
  return -1;
}

void AmpElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  memset(out, 0, sizeof(float) * numSamples * channelCount);
  for (uint32_t i = 0; i < numInputs; i++) {
    const float *inBuffer = inputs[i];
    for (uint32_t s = 0; s < numSamples * channelCount; s++) {
      out[s] += inBuffer[s] * masterAmp * channelAmp;
    }
  }
};
