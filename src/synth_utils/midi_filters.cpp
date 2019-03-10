#include "./midi_filters.h"

std::function<bool(const snd_seq_event_t*)> channelFilter(unsigned char channel) {
  return [=](const snd_seq_event_t* event) {
    return event->type >= SND_SEQ_EVENT_NOTE &&
           event->type <= SND_SEQ_EVENT_KEYSIGN &&
           event->data.control.channel == channel;
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

std::function<bool(const snd_seq_event_t*)> controlFilter(uint32_t param) {
  return [param](const snd_seq_event_t* event) {
    return event->type == SND_SEQ_EVENT_CONTROLLER &&
           event->data.control.param == param;
  };
}

int controlMap(const snd_seq_event_t* event) {
  return event->data.control.value;
}
