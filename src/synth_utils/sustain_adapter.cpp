#include "./sustain_adapter.h"

using namespace std;

void SustainAdapter::noteOnEvent(unsigned char note, unsigned char vel) {
  unsigned char adjustedVelocity = vel;
  heldNotes.insert(note);

  if (sustained) {
    if (sustainPlays.count(note) > 0) {
      sustainNoteOffEvent(note);
      // Todo: replaying the note with the higher of the two velocities
      // works well on average, but is unnatural for light playing.
      adjustedVelocity = max(adjustedVelocity, sustainPlays[note]);
    }
    sustainPlays[note] = adjustedVelocity;
  } else {
    if (heldPlays.count(note) > 0) {
      sustainNoteOffEvent(note);
    }
    heldPlays[note] = adjustedVelocity;
  }

  sustainNoteOnEvent(note, adjustedVelocity);
}

void SustainAdapter::noteOffEvent(unsigned char note) {
  heldNotes.erase(note);
  if (heldPlays.count(note) > 0) {
    sustainNoteOffEvent(note);
    heldPlays.erase(note);
    heldPlays.erase(note);
  }
}

void SustainAdapter::sustainOnEvent() {
  // in case of multiple sustainOn events
  for (const auto& it: sustainPlays) {
    sustainNoteOffEvent(it.second);
  }

  sustainPlays = heldPlays;
  heldPlays.clear();
  sustained = true;
}

void SustainAdapter::sustainOffEvent() {
  for (const auto& it: sustainPlays) {
    unsigned char note = it.first;

    // transfer any still held notes from sustain to held
    if (heldNotes.count(note) == 0) {
      sustainNoteOffEvent(note);
    } else {
      unsigned char vel = it.second;
      heldPlays.emplace(note, vel);
    }
  }

  sustainPlays.clear();
  sustained = false;
}
