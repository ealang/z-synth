#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdio.h>
#include <sstream>
#include <exception>
#include <tuple>
#include <memory>
#include <unordered_map>
#include <vector>
#include "./plan_execution.h"
#include "./pipeline_element.h"

template <typename T>
struct ExecutionStep {
  std::shared_ptr<AudioElement<T>> elem;
  std::vector<const T*> in;
  T* out;
};

/**
 * Given a DAG of elements, construct a meta element that treats the
 * entire DAG as a single element.
 */
template <typename T>
class Pipeline: public MidiAudioElement<T> {
  std::vector<std::shared_ptr<MidiListener>> midiElems;
  std::vector<ExecutionStep<T>> steps;

  uint32_t bufferSize;
  std::vector<T> buffer;
  int outputStep = -1;

  public:
  Pipeline(
    uint32_t bufferSize,
    uint32_t channelCount,
    std::unordered_map<std::string, std::shared_ptr<AudioElement<T>>> audioElems,
    std::vector<std::shared_ptr<MidiListener>> midiElems,
    std::string outputName,
    connections_t connections
  ): midiElems(midiElems), bufferSize(bufferSize) {

    plan_t plan = planExecution(connections);

    uint32_t numBuffers = countBuffersInPlan(plan);
    buffer = std::vector<T>(numBuffers * bufferSize * channelCount);

    uint32_t stepIndex = 0;
    for (auto step: plan) {
      std::string name = std::get<0>(step);
      std::set<uint32_t> inputBuffers = std::get<1>(step);
      if (name == outputName) {
        outputStep = stepIndex;
      }
      uint32_t outputBuffer = std::get<2>(step);

      uint32_t numInputs = inputBuffers.size();
      if (numInputs != audioElems[name]->nInputs()) {
        fprintf(
          stderr,
          "Warning: element \"%s\" recieved %d connections but expected %d\n",
          name.c_str(),
          numInputs,
          audioElems[name]->nInputs()
        );
      }

      std::vector<const T*> ins;
      for (auto bufferNum: inputBuffers) {
        ins.push_back(
          buffer.data() + bufferNum * bufferSize * channelCount
        );
      }

      steps.push_back(ExecutionStep<T> {
        audioElems[name],
        ins,
        buffer.data() + outputBuffer * bufferSize * channelCount
      });
      stepIndex++;
    }

    if (outputStep < 0) {
      std::ostringstream str;
      str << "Output element \"" << outputName << "\" was not found in pipeline";
      throw std::runtime_error(str.str());
    }
  }

  void generate(
    uint32_t nSamples,
    T* out,
    const T** in = nullptr
  ) {
    if (nSamples > bufferSize) {
      throw std::runtime_error("Asked for too many samples");
    }
    if (in != nullptr) {
      throw std::runtime_error("Pipeline was given an audio input");
    }

    // set user's output buffer as final output and run stages
    steps[outputStep].out = out;
    for (const auto& step: steps) {
      const T** ins = const_cast<const T**>(step.in.data());
      step.elem->generate(nSamples, step.out, ins);
    } 
  }

  void noteOnEvent(uint8_t note, uint8_t vel) {
    for (const auto& listener: midiElems) {
      listener->noteOnEvent(note, vel);
    }
  }

  void noteOffEvent(uint8_t note) {
    for (const auto& listener: midiElems) {
      listener->noteOffEvent(note);
    }
  }

  void sustainOnEvent() {
    for (const auto& listener: midiElems) {
      listener->sustainOnEvent();
    }
  }

  void sustainOffEvent() {
    for (const auto& listener: midiElems) {
      listener->sustainOffEvent();
    }
  }

  void channelPressureEvent(uint8_t val) {
    for (const auto& listener: midiElems) {
      listener->channelPressureEvent(val);
    }
  }
};

#endif
