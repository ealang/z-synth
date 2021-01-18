#include <string.h>
#include "./build_midi_event.h"

snd_seq_event_t buildNoteOnEvent(unsigned char note, unsigned char vel)
{
  snd_seq_event_t event;
  memset(&event, 0, sizeof(snd_seq_event_t));
  event.type = SND_SEQ_EVENT_NOTEON;
  event.data.note.note = note;
  event.data.note.velocity = vel;
  return event;
}

snd_seq_event_t buildNoteOffEvent(unsigned char note)
{
  snd_seq_event_t event;
  memset(&event, 0, sizeof(snd_seq_event_t));
  event.type = SND_SEQ_EVENT_NOTEOFF;
  event.data.note.note = note;
  return event;
}
