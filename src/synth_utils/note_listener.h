#ifndef NOTE_LISTENER_H
#define NOTE_LISTENER_H

#include "../pipeline/pipeline_element.h"

/* Provide callbacks for midi note and sustain pedal events. */
class NoteListener: public MidiListener {
  Rx::subscription sub1, sub2;

public:
  ~NoteListener();
  void injectMidi(Rx::observable<const snd_seq_event_t*>) override;

protected:
  virtual void noteOnEvent(unsigned char note, unsigned char vel) = 0;
  virtual void noteOffEvent(unsigned char note) = 0;
  virtual void sustainOnEvent();
  virtual void sustainOffEvent();
};

#endif