#ifndef GENERATOR_ELEMENT_H
#define GENERATOR_ELEMENT_H

#include <cstdint> 
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../pipeline/pipeline_element.h"
#include "./note_synth.h"

class GeneratorElement: public MidiAudioElement<float> {
  uint32_t sampleRateHz;
  uint32_t channelCount;

  std::unordered_set<unsigned char> sustainedNotes;
  std::unordered_set<unsigned char> heldNotes;

  std::unordered_map<unsigned char, int> noteSynths;
  std::unordered_map<uint32_t, std::shared_ptr<NoteSynth>> synths;

  bool sustained=false;
  uint32_t nextId = 0;

public:
  GeneratorElement(uint32_t sampleRateHz, uint32_t channelCount);

  void noteOnEvent(unsigned char note, unsigned char vel);
  void noteOffEvent(unsigned char note);
  void sustainOnEvent();
  void sustainOffEvent();

  void generate(uint32_t nSamples, float* out, const float** in);
};

#endif
