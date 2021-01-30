#include "./polyphony_partitioning.h"

using namespace std;

PolyphonyPartitioning::PolyphonyPartitioning(
  uint32_t numVoices
): numVoices(numVoices) {
  for (uint32_t i = 0; i < numVoices; ++i) {
    lruVoices.emplace_back(i);
    voiceToNote.emplace_back(0);
  }
}

uint32_t PolyphonyPartitioning::voiceCount() const {
  return numVoices;
}

uint32_t PolyphonyPartitioning::onNoteOnEvent(unsigned char note) {
  auto existingVoice = noteToVoice.find(note);
  if (existingVoice != noteToVoice.end()) {
    // Note: this shouldn't happen under normal circumstances
    return existingVoice->second;
  }

  heldNotes.emplace(note);

  uint32_t voice_id = lruVoices.back();
  lruVoices.pop_back();
  lruVoices.insert(lruVoices.begin(), voice_id);

  auto cur_note_id = voiceToNote[voice_id];
  if (cur_note_id != 0) {
    noteToVoice.erase(cur_note_id);
  }

  voiceToNote[voice_id] = note;
  noteToVoice[note] = voice_id;

  return voice_id;
}

uint32_t PolyphonyPartitioning::processNoteOff(unsigned char note) {
  const auto it = noteToVoice.find(note);
  if (it != noteToVoice.cend()) {
    auto voice_id = it->second;
    noteToVoice.erase(it);

    voiceToNote[voice_id] = 0;

    return voice_id;
  }

  return -1;
}

uint32_t PolyphonyPartitioning::onNoteOffEvent(unsigned char note) {
  heldNotes.erase(note);
  if (!sustainHeld) {
    return processNoteOff(note);
  }
  return -1;
}

void PolyphonyPartitioning::onSustainOnEvent() {
  sustainHeld = true;
}

// Given sustain off event, get list of now off voice channels.
vector<uint32_t> PolyphonyPartitioning::onSustainOffEvent() {
  sustainHeld = false;

  auto notes = vector<unsigned char>();
  for (const auto& it: noteToVoice) {
    notes.emplace_back(it.first);
  }

  vector<uint32_t> voices;
  for (const auto& note: notes) {
    if (!heldNotes.count(note)) {
      voices.emplace_back(processNoteOff(note));
    }
  }

  return voices;
}
