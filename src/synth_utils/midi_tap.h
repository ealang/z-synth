#ifndef MIDI_TAP_H
#define MIDI_TAP_H

#include <alsa/seq_event.h>
#include "./rx_include.h"

Rx::subscription midiTapSubscription(Rx::observable<const snd_seq_event_t*> midi);

#endif
