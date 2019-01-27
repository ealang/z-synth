#ifndef LOOPS_H
#define LOOPS_H

#include <alsa/asoundlib.h>

class MidiMux;

int audio_loop(MidiMux *mux, snd_pcm_t *handle, int period_size);
int midi_loop(snd_seq_t *seq_handle, MidiMux *mux);

#endif
