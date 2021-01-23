#include "./midi_note_listener.h"
#include "./midi_filters.h"

using namespace std;

MidiNoteListener::~MidiNoteListener() {
  sub1.unsubscribe();
  sub2.unsubscribe();
}

void MidiNoteListener::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  sub1 = midi
    | Rx::filter(noteFilter)
    | Rx::subscribe<const snd_seq_event_t*>(
        [this](const snd_seq_event_t* event) {
          bool noteOn;
          unsigned char note, vel;
          std::tie(noteOn, note, vel) = noteMap(event);
          if (!noteOn || vel == 0) {
            onNoteOffEvent(note);
          } else {
            onNoteOnEvent(note, vel);
          }
        }
      );

  sub2 = midi
    | Rx::filter(controlFilter(MIDI_PARAM_SUSTAIN))
    | Rx::map(controlMap)
    | Rx::subscribe<int>(
        [this](int amount) {
          if (amount == 0) {
            onSustainOffEvent();
          } else {
            onSustainOnEvent();
          }
        }
      );
}
