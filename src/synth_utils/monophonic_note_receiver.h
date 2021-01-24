#ifndef SUSTAINED_NOTE_RECEIVER_H
#define SUSTAINED_NOTE_RECEIVER_H

#include "./note_receiver.h"

// Emit a set of virtual note on/off events that take sustain
// pedal into account.
class MonophonicNoteReceiver: public NoteReceiver {
  bool noteIsHeld = false;
  bool sustainIsHeld = false;

protected:
  virtual void onVirtualNoteOnEvent(unsigned char note, unsigned char velocity) = 0;
  virtual void onVirtualNoteOffEvent() = 0;

public:
  virtual ~MonophonicNoteReceiver();

  void onNoteOnEvent(unsigned char note, unsigned char vel) override;
  void onNoteOffEvent(unsigned char note) override;
  void onSustainOnEvent() override;
  void onSustainOffEvent() override;
};

#endif
