#ifndef NOTE_RECEIVER_H
#define NOTE_RECEIVER_H

class NoteReceiver {
public:
  virtual void onNoteOnEvent(unsigned char note, unsigned char vel) = 0;
  virtual void onNoteOffEvent(unsigned char note) = 0;
  virtual void onSustainOnEvent() = 0;
  virtual void onSustainOffEvent() = 0;
};

#endif
