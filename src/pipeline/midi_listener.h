#ifndef MIDI_LISTENER_H
#define MIDI_LISTENER_H

#include <alsa/seq_event.h>
#include "./rx_include.h"

class MidiListener {
public:
  virtual void injectMidi(Rx::observable<const snd_seq_event_t*>) = 0;
};

#endif
