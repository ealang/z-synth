#ifndef PIPELINE_ELEMENT_H
#define PIPELINE_ELEMENT_H

#include <cstdint>
#include "./midi_listener.h"

template <typename T>
using inputs_t = const T* const *;

template <typename T>
class AudioElement {
public:
  virtual uint32_t maxInputs() = 0;
  virtual void generate(
    uint32_t numSamples,
    T* out,
    uint32_t numInputs,
    inputs_t<T> inputs
  ) = 0;
};

template <typename T>
class MidiAudioElement: public AudioElement<T>, public MidiListener {
};

#endif
