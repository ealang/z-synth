#ifndef PIPELINE_BUILDER_H
#define PIPELINE_BUILDER_H

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "./pipeline.h"
#include "./midi_listener.h"
#include "./pipeline_element.h"

/* Allow at-runtime construction of an audio pipeline. Elements can
 * be added and connected together. Ultimately the pipeline is created
 * by calling `build`.
 */
template <typename T>
class PipelineBuilder {
  std::unordered_map<std::string, std::shared_ptr<AudioElement<T>>> audioElems;
  std::vector<std::shared_ptr<MidiListener>> midiElems;
  std::string outputElem;
  connections_t connections;

  void reset() {
    audioElems.clear();
    midiElems.clear();
    connections.clear();
  }

public:
  void registerElem(std::string name, std::shared_ptr<AudioElement<T>> elem) {
    audioElems[name] = elem;
    if (connections.count(name) == 0) {
      connections.emplace(name, std::set<std::string>());
    }
  }

  void registerMidi(std::shared_ptr<MidiListener> elem) {
    midiElems.push_back(elem);
  }

  void connectElems(std::string from, std::string to) {
    connections[from].insert(to);
  }

  void setOutputElem(std::string name) {
    outputElem = name;
  }

  std::shared_ptr<MidiAudioElement<T>> build(uint32_t bufferSize, uint32_t channelCount) {
    auto pipeline = std::make_shared<Pipeline<T>>(
      bufferSize,
      channelCount,
      audioElems,
      midiElems,
      outputElem,
      connections
    );
    reset();
    return pipeline;
  }
};

#endif
