#ifndef NOTE_LISTENER_H
#define NOTE_LISTENER_H

#include "../rx_include.h"
#include <alsa/seq_event.h>

/* Provide callbacks for midi note and sustain pedal events. */
class NoteListener {
  Rx::subscription sub1, sub2;

public:
  virtual ~NoteListener();
  void injectMidi(Rx::observable<const snd_seq_event_t*>);

protected:
  virtual void noteOnEvent(unsigned char note, unsigned char vel);
  virtual void noteOnEvent(const snd_seq_event_t* event);

  virtual void noteOffEvent(unsigned char note);
  virtual void noteOffEvent(const snd_seq_event_t* event);

  virtual void sustainOnEvent();
  virtual void sustainOffEvent();
};

#endif
