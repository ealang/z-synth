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

  // Maintain a set of held and sustained data, and bounce
  // notes back and forth based on the state of the pedal.
  std::unordered_map<unsigned char, uint32_t> heldSynths;
  std::unordered_map<unsigned char, uint32_t> sustainSynths;
  std::unordered_map<unsigned char, unsigned char> heldVelocity;
  std::unordered_map<unsigned char, unsigned char> sustainVelocity;

  std::unordered_map<uint32_t, std::shared_ptr<NoteSynth>> synths;
  std::unordered_set<unsigned char> heldNotes;
  bool sustained = false;
  uint32_t nextId = 0;
  uint32_t createId();

public:
  GeneratorElement(uint32_t sampleRateHz, uint32_t channelCount);

  void noteOnEvent(unsigned char note, unsigned char vel);
  void noteOffEvent(unsigned char note);
  void sustainOnEvent();
  void sustainOffEvent();

  uint32_t maxInputs() override;
  void generate(uint32_t numSamples, float* out, uint32_t, inputs_t<float>) override;
};

#endif
