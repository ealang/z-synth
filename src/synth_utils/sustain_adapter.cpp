#include "./sustain_adapter.h"
#include "./midi_filters.h"

using namespace std;

SustainAdapter::~SustainAdapter() {
  sub1.unsubscribe();
  sub2.unsubscribe();
}

void SustainAdapter::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  sub1 = midi
    | Rx::filter(noteFilter)
    | Rx::map(noteMap)
    | Rx::subscribe<tuple<bool, uint8_t, uint8_t>>(
        [this](tuple<bool, uint8_t, uint8_t> event) {
          auto noteOn = get<0>(event);
          auto note = get<1>(event);
          auto vel = get<2>(event);
          if (!noteOn || vel == 0) {
            noteOffEvent(note);
          } else {
            noteOnEvent(note, vel);
          }
        }
      );

  sub2 = midi
    | Rx::filter(sustainFilter)
    | Rx::map(sustainMap)
    | Rx::subscribe<uint8_t>(
        [this](uint8_t amount) {
          if (amount == 0) {
            sustainOffEvent();
          } else {
            sustainOnEvent();
          }
        }
      );
}

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
