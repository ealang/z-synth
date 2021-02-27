#ifndef THREADED_MIXER_ELEMENT_H
#define THREADED_MIXER_ELEMENT_H

#include "../pipeline/pipeline_element.h"
#include "./mixer_element.h"

#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

class SharedState;

// Element that executes a list of 0 input audio elements using a thread pool.
// Results are combined together.
class ThreadedMixerElement: public AudioElement<float> {
  std::vector<std::thread> _threads;
  std::vector<float> _outputBuffers;
  std::vector<std::shared_ptr<AudioElement<float>>> _elements;

  std::shared_ptr<SharedState> _state;

  std::vector<float*> _mixerInputs;
  MixerElement _mixer;

public:
  ThreadedMixerElement(
    uint32_t bufferSampleCount,
    uint32_t numThreads,
    std::vector<std::shared_ptr<AudioElement<float>>> elements,
    float amp
  );
  ~ThreadedMixerElement();

  uint32_t maxInputs() const override;

  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
