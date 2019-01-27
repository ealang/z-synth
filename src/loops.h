#ifndef LOOPS_H
#define LOOPS_H

#include <alsa/asoundlib.h>

class MidiMux;

int audioLoop(snd_pcm_t *const audioDevice, MidiMux *const mux, int period_size);
int midiLoop(snd_seq_t *const midiDevice, MidiMux *const mux);

#endif
