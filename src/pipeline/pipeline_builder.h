#ifndef PIPELINE_BUILDER_H
#define PIPELINE_BUILDER_H

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "./pipeline.h"
#include "./midi_listener.h"
#include "./pipeline_element.h"

template <typename T>
class PipelineBuilder {
  std::unordered_map<std::string, std::shared_ptr<AudioElement<T>>> audioElems;
  std::vector<std::shared_ptr<MidiListener>> midiElems;
  std::string outputElem;
  connections_t connections;

public:
  void registerElem(std::string name, std::shared_ptr<AudioElement<T>> elem) {
    audioElems[name] = elem;
    if (connections.count(name) == 0) {
      connections.emplace(name, std::set<std::string>());
    }
  }

  void registerElem(std::string name, std::shared_ptr<MidiAudioElement<T>> elem) {
    midiElems.push_back(elem);
    registerElem(name, static_cast<std::shared_ptr<AudioElement<T>>>(elem));
  }

  void connectElems(std::string from, std::string to) {
    connections[from].insert(to);
  }

  void setOutputElem(std::string name) {
    outputElem = name;
  }

  std::shared_ptr<MidiAudioElement<T>> build(uint32_t bufferSize, uint32_t channelCount) {
    return std::make_shared<Pipeline<T>>(
      bufferSize,
      channelCount,
      audioElems,
      midiElems,
      outputElem,
      connections
    );
  }
};

#endif
