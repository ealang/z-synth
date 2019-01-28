#ifndef NOTE_MUX_H
#define NOTE_MUX_H

#include <cstdint> 
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "note_synth.h"
#include "filter.h"

class MidiMux {
  uint32_t sampleRateHz;
  uint32_t channelCount;

  std::unordered_set<unsigned char> sustainedNotes;
  std::unordered_set<unsigned char> heldNotes;

  std::unordered_map<unsigned char, int> noteSynths;
  std::unordered_map<uint32_t, std::shared_ptr<NoteSynth>> synths;
  std::vector<Filter> filters;

  std::mutex lock;
  bool sustained=false;
  uint32_t nextId = 0;

public:
  MidiMux(uint32_t sampleRateHz, uint32_t channelCount);

  void noteOnEvent(unsigned char note, unsigned char vel);
  void noteOffEvent(unsigned char note);
  void sustainOnEvent();
  void sustainOffEvent();
  void channelPressureEvent(unsigned char pressure);
  void generate(uint32_t nSamples, float* buffer);
};

#endif
