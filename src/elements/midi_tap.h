#ifndef MIDI_TAP_H
#define MIDI_TAP_H

#include "../pipeline/midi_listener.h"

class MidiTap: public MidiListener {
  Rx::subscription sub;
public:
  ~MidiTap();
  void injectMidi(Rx::observable<const snd_seq_event_t*>) override;
};

#endif
