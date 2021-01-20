#include "./note_listener.h"
#include "./midi_filters.h"

using namespace std;

NoteListener::~NoteListener() {
  sub1.unsubscribe();
  sub2.unsubscribe();
}

void NoteListener::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  sub1 = midi
    | Rx::filter(noteFilter)
    | Rx::subscribe<const snd_seq_event_t*>(
        [this](const snd_seq_event_t* event) {
          bool noteOn;
          unsigned char note, vel;
          std::tie(noteOn, note, vel) = noteMap(event);
          if (!noteOn || vel == 0) {
            noteOffEvent(note);
            noteOffEvent(event);
          } else {
            noteOnEvent(note, vel);
            noteOnEvent(event);
          }
        }
      );

  sub2 = midi
    | Rx::filter(controlFilter(MIDI_PARAM_SUSTAIN))
    | Rx::map(controlMap)
    | Rx::subscribe<int>(
        [this](int amount) {
          if (amount == 0) {
            sustainOffEvent();
          } else {
            sustainOnEvent();
          }
        }
      );
}

void NoteListener::noteOnEvent(unsigned char, unsigned char) {
  // default no-op handler
}

void NoteListener::noteOnEvent(const snd_seq_event_t*) {
  // default no-op handler
}

void NoteListener::noteOffEvent(unsigned char) {
  // default no-op handler
}

void NoteListener::noteOffEvent(const snd_seq_event_t*) {
  // default no-op handler
}

void NoteListener::sustainOnEvent() {
  // default no-op handler
}

void NoteListener::sustainOffEvent() {
  // default no-op handler
}
