#ifndef PIPELINE_ELEMENT_H
#define PIPELINE_ELEMENT_H

#include <cstdint>

template <typename T>
using inputs_t = const T* const *;

template <typename T>
class AudioElement {
public:
  virtual ~AudioElement() {}
  virtual uint32_t maxInputs() const = 0;
  virtual void generate(
    uint32_t numSamples,
    T* out,
    uint32_t numInputs,
    inputs_t<T> inputs
  ) = 0;
};

#endif
