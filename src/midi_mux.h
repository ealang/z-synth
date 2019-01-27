#ifndef NOTE_MUX_H
#define NOTE_MUX_H

#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "audio_param.h"
#include "note_synth.h"

class MidiMux {
  std::unordered_set<unsigned char> sustainedNotes;
  std::unordered_set<unsigned char> heldNotes;

  std::unordered_map<unsigned char, int> noteSynths;
  std::unordered_map<int, std::shared_ptr<NoteSynth>> synths;

  AudioParam audioParam;
  std::mutex lock;
  bool sustained=false;
  uint32_t sampleRateHz;
  uint64_t nextId = 0;

public:

  MidiMux(AudioParam audioParam);

  void noteOnEvent(unsigned char note, unsigned char vel);
  void noteOffEvent(unsigned char note);
  void sustainOnEvent();
  void sustainOffEvent();
  void channelPressureEvent(unsigned char pressure);
  void generate(sample_t* buffer, int count);
};

#endif
