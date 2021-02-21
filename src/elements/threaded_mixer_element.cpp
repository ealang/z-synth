#include "./threaded_mixer_element.h"

#include <atomic>
#include <condition_variable>
#include <functional>

static constexpr uint32_t initGenerationFlag = 0;

struct SharedState {
  std::condition_variable workerCv;
  std::condition_variable producerCv;
  std::vector<std::function<void()>> work;
  bool shouldExit;
  std::atomic<uint32_t> generationFlag;
  std::atomic<uint32_t> workRemaining;

  SharedState():
    shouldExit(false),
    generationFlag(initGenerationFlag),
    workRemaining(0) {}
};

static void workerThreadMain(uint32_t threadId, uint32_t numThreads, SharedState* state) {
  std::mutex mutex;

  uint32_t lastGenerationFlag = initGenerationFlag;

  auto workReady = [state, &lastGenerationFlag]() {
    return state->generationFlag != lastGenerationFlag || state->shouldExit;
  };

  while (!state->shouldExit) {
    // Wait for work
    {
      std::unique_lock<std::mutex> lock(mutex);
      state->workerCv.wait(lock, workReady);
    }
    if (state->shouldExit) {
      break;
    }
    lastGenerationFlag = state->generationFlag;

    // Do work
    const auto workSize = state->work.size();
    for (uint32_t i = threadId; i < workSize; i += numThreads) {
      state->work[i]();
    }

    // Notify requester
    if (state->workRemaining == 0) {
      throw std::runtime_error("invalid state");
    }
    if (--state->workRemaining == 0) {
      state->producerCv.notify_one();
    }
  }
}

ThreadedMixerElement::ThreadedMixerElement(
  uint32_t bufferSampleCount,
  uint32_t numThreads,
  std::vector<std::shared_ptr<AudioElement<float>>> elements,
  float amp
): _outputBuffers(elements.size() * bufferSampleCount, 0),
   _elements(elements),
   _state(std::make_shared<SharedState>()),
   _ampInputs(elements.size(), 0),
   _amp(amp)
{

  for (uint32_t i = 0; i < elements.size(); ++i) {
    float *outputBuffer = &_outputBuffers.data()[i * bufferSampleCount];
    _ampInputs[i] = outputBuffer;

    auto element = elements[i];
    _state->work.emplace_back([element, bufferSampleCount, outputBuffer]() {
      element->generate(bufferSampleCount, outputBuffer, 0, nullptr);
    });
  }

  for (uint32_t i = 0; i < numThreads; ++i) {
    _threads.emplace_back(workerThreadMain, i, numThreads, _state.get());
  }
}

ThreadedMixerElement::~ThreadedMixerElement() {
  _state->shouldExit = true;
  _state->workerCv.notify_all();
  for (auto &thread: _threads) {
    thread.join();
  }
}

uint32_t ThreadedMixerElement::maxInputs() const {
  return 0;
}

void ThreadedMixerElement::setAmp(float amp) {
  _amp.setAmp(amp);
}

void ThreadedMixerElement::generate(uint32_t numSamples, float* out, uint32_t, inputs_t<float>) {
  const uint32_t numElements = _elements.size();

  _state->workRemaining = _threads.size();
  ++_state->generationFlag;

  _state->workerCv.notify_all();

  {
    std::unique_lock<std::mutex> lock(mutex);
    _state->producerCv.wait(lock, [this]() {
      return _state->workRemaining == 0;
    });
  }

  _amp.generate(numSamples, out, numElements, _ampInputs.data());
};
