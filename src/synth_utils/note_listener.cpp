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

void NoteListener::sustainOnEvent() {
  // default no-op handler
}

void NoteListener::sustainOffEvent() {
  // default no-op handler
}
