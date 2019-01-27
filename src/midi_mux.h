#ifndef NOTE_MUX_H
#define NOTE_MUX_H

#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "types.h"
#include "note_synth.h"

class MidiMux {
  std::unordered_map<unsigned char, int> notesMap;
  std::unordered_map<int, std::shared_ptr<NoteSynth>> synths;
  std::mutex lock;
  uint64_t nextId = 0;
  uint32_t sampleRateHz;


public:

  MidiMux(uint32_t sampleRateHz);

  void noteOnEvent(unsigned char note, unsigned char vel);
  void noteOffEvent(unsigned char note);
  void channelPressureEvent(unsigned char pressure);
  void generate(sample_t* buffer, int count);
};

#endif
