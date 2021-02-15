#ifndef PIPELINE_H
#define PIPELINE_H

#define PIPELINE_H_PROFILING 0

#include "./plan_execution.h"
#include "./pipeline_element.h"

#if PIPELINE_H_PROFILING
#include "../metric.h"
#endif

#include <sstream>
#include <exception>
#include <memory>
#include <unordered_map>
#include <vector>

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

  #if PIPELINE_H_PROFILING
  std::vector<std::string> stepNames;
  std::vector<Metric> stepMetrics;
  const uint32_t metricLength = 1000;
  clock_t nextReportTime = 0;
  clock_t reportInterval = CLOCKS_PER_SEC * 5;
  #endif

  uint32_t bufferSize;
  std::vector<T> buffer;
  int outputStep = -1;

  #if PIPELINE_H_PROFILING
  void reportMetrics() const {
    auto namesIt = stepNames.cbegin();
    auto metricsIt = stepMetrics.cbegin();
    while (namesIt != stepNames.cend()) {
      auto report = metricsIt->report();

      printf("%s, %.3f, %.3f, %.3f\n",
        namesIt->c_str(),
        report.max * 1000,
        report.p99 * 1000,
        report.p75 * 1000
      );

      ++namesIt;
      ++metricsIt;
    }
  }
  #endif

public:
  Pipeline(
    uint32_t bufferSize,
    const std::unordered_map<std::string, std::shared_ptr<AudioElement<T>>>& audioElems,
    std::string outputName,
    const connections_t& connections
  ): bufferSize(bufferSize) {
    if (audioElems.size() == 0) {
      throw std::runtime_error("No elements were provided");
    }

    for (const auto& elemName: allNodes(connections)) {
      if (audioElems.count(elemName) == 0) {
        std::ostringstream str;
        str << "Element \"" << elemName << "\" has not been registered";
        throw std::runtime_error(str.str());
      }
    }

    for (const auto& elemName: findTerminalNodes(connections)) {
      if (elemName != outputName) {
        std::cerr << "Warning: Element \"" << elemName << "\" has no path to the output" << std::endl;
      }
    }

    const plan_t plan = planExecution(connections);
    const uint32_t numBuffers = countBuffersInPlan(plan);
    buffer.resize(numBuffers * bufferSize);

    uint32_t stepIndex = 0;
    for (const auto& step: plan) {
      const std::string& name = std::get<0>(step);
      const std::vector<uint32_t>& inputBuffers = std::get<1>(step);
      if (name == outputName) {
        outputStep = stepIndex;
      }
      const uint32_t outputBuffer = std::get<2>(step);

      const uint32_t numInputs = inputBuffers.size();
      const uint32_t maxInputs = audioElems.find(name)->second->maxInputs();
      if (numInputs > maxInputs) {
        std::ostringstream str;
        str << "Element \"" << name << "\" received " << numInputs << " input(s) but supports a max of " << maxInputs;
        throw std::runtime_error(str.str());
      }

      std::vector<const T*> ins;
      for (const auto bufferNum: inputBuffers) {
        ins.push_back(
          bufferNum == NULL_BUFFER_NUM
            ? nullptr
            : buffer.data() + bufferNum * bufferSize
        );
      }

      steps.emplace_back(ExecutionStep<T> {
        audioElems.find(name)->second,
        ins,
        buffer.data() + outputBuffer * bufferSize
      });

      #if PIPELINE_H_PROFILING
      stepNames.emplace_back(name);
      stepMetrics.emplace_back(metricLength);
      #endif

      stepIndex++;
    }

    if (outputStep < 0) {
      std::ostringstream str;
      str << "Output element \"" << outputName << "\" was not found in pipeline";
      throw std::runtime_error(str.str());
    }
  }

  uint32_t maxInputs() const override {
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

    #if PIPELINE_H_PROFILING
    auto metricsIt = stepMetrics.begin();
    #endif

    // set user's output buffer as final output and run stages
    steps[outputStep].out = out;
    for (const auto& step: steps) {

      #if PIPELINE_H_PROFILING
      auto timer = stopWatchSeconds();
      #endif

      inputs_t<T> ins = step.in.data();
      step.elem->generate(numSamples, step.out, step.in.size(), ins);

      #if PIPELINE_H_PROFILING
      metricsIt->record(timer());
      ++metricsIt;
      #endif
    } 

    #if PIPELINE_H_PROFILING
    clock_t time = clock();
    if (time >= nextReportTime) {
      nextReportTime = time + reportInterval;
      reportMetrics();
    }
    #endif
  }
};

#endif
