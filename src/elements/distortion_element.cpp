#include <cstring>
#include <cmath>
#include "./distortion_element.h"
#include "../synth_utils/midi_filters.h"

DistortionElement::DistortionElement(AudioParams params):
  params(params) {}

void DistortionElement::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  sub = midi
    | Rx::filter(controlFilter(MIDI_PARAM_MOD_WHEEL))
    | Rx::map(controlMap)
    | Rx::subscribe<int>([this](int value) {
        onModChanged(value);
      });
}

DistortionElement::~DistortionElement() {
  sub.unsubscribe();
}

uint32_t DistortionElement::maxInputs() {
  return 1;
}

void DistortionElement::onModChanged(uint8_t value) {
  amount = value/127.;
}

void DistortionElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  uint32_t total = numSamples * params.channelCount;
  if (numInputs > 0) {
    const float* in = inputs[0];
    if (amount > 0) {
      for (uint32_t i = 0; i < total; i++) {
        out[i] = tanh(in[i] * (0.5 + amount * 4));
      }
    } else {
      memcpy(out, in, total * sizeof(float));
    }
  } else {
    memset(out, 0, total * sizeof(float));
  }
}
