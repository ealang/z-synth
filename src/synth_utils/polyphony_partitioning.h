#ifndef POLYPHONY_PARTITIONING_H
#define POLYPHONY_PARTITIONING_H

#include <cstdint>
#include <set>
#include <unordered_map>
#include <vector>

// Partition a stream of note on/off events per voice/generator.
class PolyphonyPartitioning {
  uint32_t numVoices;
  std::unordered_map<unsigned char, uint32_t> noteToVoice;
  std::vector<uint32_t> voiceToNote;
  std::vector<uint32_t> lruVoices;
  std::set<uint32_t> heldNotes;
  bool sustainHeld = false;

  uint32_t processNoteOff(unsigned char note);

public:
  PolyphonyPartitioning(uint32_t numVoices);

  uint32_t voiceCount() const;

  // Given next noteOn event, get the assigned voice channel.
  uint32_t onNoteOnEvent(unsigned char note);
  // Given next noteOff event, get the assigned voice channel.
  // Returns -1 if cannot be matched to a voice channel.
  uint32_t onNoteOffEvent(unsigned char note);

  void onSustainOnEvent();
  // Given sustain off event, get list of now off voice channels.
  std::vector<uint32_t> onSustainOffEvent();
};

#endif
