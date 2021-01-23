#ifndef POLYPHONY_PARTITIONING_H
#define POLYPHONY_PARTITIONING_H

#include <cstdint>
#include <vector>
#include <unordered_map>

// Partition a stream of note on/off events per voice/generator.
class PolyphonyPartitioning {
  uint32_t numVoices;
  std::unordered_map<unsigned char, uint32_t> noteToVoice;
  std::vector<uint32_t> voiceToNote;
  std::vector<uint32_t> lruVoices;

public:
  PolyphonyPartitioning(uint32_t numVoices);

  uint32_t voiceCount() const;

  // Given next noteOn event, get the assigned voice channel.
  uint32_t onNoteOnEvent(unsigned char note);
  // Given next noteOff event, get the assigned voice channel.
  // Returns -1 if cannot be matched to a voice channel.
  uint32_t onNoteOffEvent(unsigned char note);
};

#endif
