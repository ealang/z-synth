#include "./midi_filters.h"

std::function<bool(const snd_seq_event_t*)> channelFilter(unsigned char channel) {
  return [=](const snd_seq_event_t* event) {
    return event->data.control.channel == channel;
  };
}

bool noteFilter(const snd_seq_event_t* event) {
  switch (event->type) {
    case SND_SEQ_EVENT_NOTEON:
    case SND_SEQ_EVENT_NOTEOFF: 
      return true;
    default:
      return false;
  }
}

std::tuple<bool, uint8_t, uint8_t> noteMap(const snd_seq_event_t* event) {
  bool isNoteOn = event->type == SND_SEQ_EVENT_NOTEON;
  return std::make_tuple(
    isNoteOn,
    event->data.note.note,
    event->data.note.velocity
  );
}

bool sustainFilter(const snd_seq_event_t* event) {
  static const unsigned char sustainControlNumber = 64;
  return event->data.control.param == sustainControlNumber;
}

uint8_t sustainMap(const snd_seq_event_t* event) {
  return event->data.control.value;
}
