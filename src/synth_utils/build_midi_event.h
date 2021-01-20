#ifndef BUILD_MIDI_EVENT_H
#define BUILD_MIDI_EVENT_H

#include <alsa/seq_event.h>

snd_seq_event_t buildNoteOnEvent(unsigned char note, unsigned char vel);
snd_seq_event_t buildNoteOffEvent(unsigned char note);
snd_seq_event_t buildSustainEvent(unsigned char value);

#endif
