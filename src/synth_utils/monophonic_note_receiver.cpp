#include "./monophonic_note_receiver.h"

MonophonicNoteReceiver::~MonophonicNoteReceiver() {}

void MonophonicNoteReceiver::onNoteOnEvent(unsigned char note, unsigned char velocity) {
  noteIsHeld = true;
  onVirtualNoteOnEvent(note, velocity);
}

void MonophonicNoteReceiver::onNoteOffEvent(unsigned char) {
  noteIsHeld = false;
  if (!sustainIsHeld) {
    onVirtualNoteOffEvent();
  }
}

void MonophonicNoteReceiver::onSustainOnEvent() {
  sustainIsHeld = true;
}

void MonophonicNoteReceiver::onSustainOffEvent() {
  sustainIsHeld = false;
  if (!noteIsHeld) {
    onVirtualNoteOffEvent();
  }
}
