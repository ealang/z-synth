#include "loops.h"
#include "midi_mux.h"

static int xrunRecovery(snd_pcm_t *const audioDevice, int err) {
  printf("stream recovery\n");
  if (err == -EPIPE) {    /* under-run */
    err = snd_pcm_prepare(audioDevice);
    if (err < 0)
      printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
    return 0;
  } else if (err == -ESTRPIPE) {
    while ((err = snd_pcm_resume(audioDevice)) == -EAGAIN)
      sleep(1);       /* wait until the suspend flag is released */
    if (err < 0) {
      err = snd_pcm_prepare(audioDevice);
      if (err < 0)
        printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
    }
    return 0;
  }
  return err;
}

int audioLoop(snd_pcm_t *const audioDevice, MidiMux *const mux, int period_size) {
  sample_t *samples = new sample_t[period_size];
  signed short *ptr;
  int err, cptr;
  while (1) {
    mux->generate(samples, period_size);
    ptr = samples;
    cptr = period_size;
    while (cptr > 0) {
      err = snd_pcm_writei(audioDevice, ptr, cptr);
      if (err == -EAGAIN)
        continue;
      if (err < 0) {
        if (xrunRecovery(audioDevice, err) < 0) {
          printf("Write error: %s\n", snd_strerror(err));
          exit(EXIT_FAILURE);
        }
        break;  /* skip one period */
      }
      ptr += err;
      cptr -= err;
    }
  }

  delete[] samples;
}

static void handleMidiEvent(MidiMux *const mux, snd_seq_t *const midiDevice) {
  snd_seq_event_t *ev;

  do {
    snd_seq_event_input(midiDevice, &ev);
    switch (ev->type) {
      case SND_SEQ_EVENT_NOTEON:
        fprintf(stderr, "Note On event on Channel %d: %d (vel %d)\n",
            ev->data.control.channel, ev->data.note.note, ev->data.note.velocity);
        if (ev->data.note.velocity == 0) {
          mux->noteOffEvent(ev->data.note.note);
        } else {
          mux->noteOnEvent(ev->data.note.note, ev->data.note.velocity);
        }
        break;        
      case SND_SEQ_EVENT_NOTEOFF: 
        fprintf(stderr, "Note Off event on Channel %d: %d\n",         
            ev->data.control.channel, ev->data.note.note);           
        mux->noteOffEvent(ev->data.note.note);
        break;        
      case SND_SEQ_EVENT_CHANPRESS:
        fprintf(stderr, "Aftertouch event on Channel %d: %d\n",         
            ev->data.control.channel, ev->data.control.value);
        mux->channelPressureEvent(ev->data.control.value);
        break;        
    }
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(midiDevice, 0) > 0);
}

int midiLoop(snd_seq_t *const midiDevice, MidiMux *const mux) {
  int npfd;
  struct pollfd *pfd;

  npfd = snd_seq_poll_descriptors_count(midiDevice, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(midiDevice, pfd, npfd, POLLIN);
  while (1) {
    if (poll(pfd, npfd, 1000000) > 0) {
      handleMidiEvent(mux, midiDevice);
    }  
  }
}
