#include "./threaded_mixer_element.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>

static constexpr uint32_t initGenerationFlag = 0;

struct SharedState {
  std::mutex workMutex;
  std::mutex resultMutex;

  std::condition_variable workCv;
  std::condition_variable resultCv;

  std::vector<std::function<void()>> work;

  bool shouldExit;
  uint32_t generationFlag;
  uint32_t workRemaining;

  SharedState():
    shouldExit(false),
    generationFlag(initGenerationFlag),
    workRemaining(0) {}
};

static void workerThreadMain(uint32_t threadId, uint32_t numThreads, SharedState* state) {

  uint32_t lastGenerationFlag = initGenerationFlag;

  auto shouldWake = [state, &lastGenerationFlag]() {
    return state->generationFlag != lastGenerationFlag || state->shouldExit;
  };

  while (!state->shouldExit) {
    // Wait for work
    {
      std::unique_lock<std::mutex> lock(state->workMutex);
      state->workCv.wait(lock, shouldWake);
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
    {
      std::unique_lock<std::mutex> lock(state->resultMutex);
      if (state->workRemaining == 0) {
        throw std::runtime_error("invalid state");
      }
      if (--state->workRemaining == 0) {
        lock.unlock();
        state->resultCv.notify_one();
      }
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
  _state->workCv.notify_all();
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

  {
    std::unique_lock<std::mutex> lock(_state->workMutex);
    _state->workRemaining = _threads.size();
    ++_state->generationFlag;
  }

  _state->workCv.notify_all();

  {
    std::unique_lock<std::mutex> lock(_state->resultMutex);
    _state->resultCv.wait(lock, [this]() {
      return _state->workRemaining == 0;
    });
  }

  _amp.generate(numSamples, out, numElements, _ampInputs.data());
};
