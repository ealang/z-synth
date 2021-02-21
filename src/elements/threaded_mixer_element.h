#ifndef THREADED_MIXER_ELEMENT_H
#define THREADED_MIXER_ELEMENT_H

#include "../pipeline/pipeline_element.h"
#include "./amp_element.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class SharedState;

// Element that executes a list of 0 input audio elements using a thread pool.
// Results are combined together.
class ThreadedMixerElement: public AudioElement<float> {
  std::vector<std::thread> _threads;
  std::vector<float> _outputBuffers;
  std::vector<std::shared_ptr<AudioElement<float>>> _elements;
  std::mutex mutex;

  std::shared_ptr<SharedState> _state;

  std::vector<float*> _ampInputs;
  AmpElement _amp;

public:
  ThreadedMixerElement(
    uint32_t bufferSampleCount,
    uint32_t numThreads,
    std::vector<std::shared_ptr<AudioElement<float>>> elements,
    float amp
  );
  ~ThreadedMixerElement();

  uint32_t maxInputs() const override;

  void setAmp(float amp);

  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
