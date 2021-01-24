#ifndef PIPELINE_H
#define PIPELINE_H

#include <iostream>
#include <sstream>
#include <exception>
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
 * Given a DAG of audio elements, construct a meta element that treats the
 * entire DAG as a single element.
 */
template <typename T>
class Pipeline: public AudioElement<T> {
  std::vector<ExecutionStep<T>> steps;

  uint32_t bufferSize;
  std::vector<T> buffer;
  int outputStep = -1;

  public:
  Pipeline(
    uint32_t bufferSize,
    std::unordered_map<std::string, std::shared_ptr<AudioElement<T>>> audioElems,
    std::string outputName,
    connections_t connections
  ): bufferSize(bufferSize) {
    plan_t plan = planExecution(connections);

    if (audioElems.size() == 0) {
      throw std::runtime_error("No elements were provided");
    }

    for (std::string elemName: allNodes(connections)) {
      if (audioElems.count(elemName) == 0) {
        std::ostringstream str;
        str << "Element \"" << elemName << "\" has not been registered";
        throw std::runtime_error(str.str());
      }
    }

    for (std::string elemName: findTerminalNodes(connections)) {
      if (elemName != outputName) {
        std::cerr << "Warning: Element \"" << elemName << "\" has no path to the output" << std::endl;
      }
    }

    uint32_t numBuffers = countBuffersInPlan(plan);
    buffer = std::vector<T>(numBuffers * bufferSize);

    uint32_t stepIndex = 0;
    for (auto step: plan) {
      std::string name = std::get<0>(step);
      std::set<uint32_t> inputBuffers = std::get<1>(step);
      if (name == outputName) {
        outputStep = stepIndex;
      }
      uint32_t outputBuffer = std::get<2>(step);

      uint32_t numInputs = inputBuffers.size();
      uint32_t maxInputs = audioElems[name]->maxInputs();
      if (numInputs > maxInputs) {
        std::ostringstream str;
        str << "Element \"" << name << "\" received " << numInputs << " input(s) but supports a max of " << maxInputs;
        throw std::runtime_error(str.str());
      }

      if (numInputs == 0 && maxInputs > 0) {
        std::cerr << "Warning: Element \"" << name << "\" received no inputs" << std::endl;
      }

      std::vector<const T*> ins;
      for (auto bufferNum: inputBuffers) {
        ins.push_back(
          buffer.data() + bufferNum * bufferSize
        );
      }

      steps.push_back(ExecutionStep<T> {
        audioElems[name],
        ins,
        buffer.data() + outputBuffer * bufferSize
      });
      stepIndex++;
    }

    if (outputStep < 0) {
      std::ostringstream str;
      str << "Output element \"" << outputName << "\" was not found in pipeline";
      throw std::runtime_error(str.str());
    }
  }

  uint32_t maxInputs() override {
    return 0;
  }

  void generate(
    uint32_t numSamples,
    T* out,
    uint32_t,
    inputs_t<T>
  ) override {
    if (numSamples > bufferSize) {
      throw std::runtime_error("Asked for too many samples");
    }

    // set user's output buffer as final output and run stages
    steps[outputStep].out = out;
    for (const auto& step: steps) {
      inputs_t<T> ins = step.in.data();
      step.elem->generate(numSamples, step.out, step.in.size(), ins);
    } 
  }
};

#endif
