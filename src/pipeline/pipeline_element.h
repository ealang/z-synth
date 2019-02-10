#ifndef PIPELINE_ELEMENT_H
#define PIPELINE_ELEMENT_H

#include <cstdint>
#include "./midi_listener.h"

template <typename T>
class AudioElement {
public:
  virtual uint32_t nInputs() const { return 0; }
  virtual void generate(
    uint32_t,
    T* out,
    const T** in = nullptr
  ) = 0;
};

template <typename T>
class MidiAudioElement: public AudioElement<T>, public MidiListener {
};

#endif
