#ifndef MIDI_NOTE_LISTENER_H
#define MIDI_NOTE_LISTENER_H

#include "../rx_include.h"
#include <alsa/seq_event.h>

/* Provide callbacks for midi note and sustain pedal events. */
class MidiNoteListener {
  Rx::subscription sub1, sub2;

public:
  virtual ~MidiNoteListener();

protected:
  void injectMidi(Rx::observable<const snd_seq_event_t*>);

  virtual void onNoteOnEvent(unsigned char note, unsigned char vel) = 0;
  virtual void onNoteOffEvent(unsigned char note) = 0;
  virtual void onSustainOnEvent() = 0;
  virtual void onSustainOffEvent() = 0;
};

#endif
