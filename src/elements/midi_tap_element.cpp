#include <string>
#include <iostream>
#include <sstream>
#include "./midi_tap_element.h"
#include "../synth_utils/midi_filters.h"

using namespace std;

static string dumpNote(snd_seq_ev_note_t* data) {
  ostringstream s;
  s << "ch: " << int(data->channel) << " "
    << "note: " << int(data->note) << " "
    << "vel: " << int(data->velocity) << " "
    << "off_vel: " << int(data->off_velocity) << " "
    << "dur: " << int(data->duration) << endl;
  return s.str();
}

static string dumpControl(snd_seq_ev_ctrl_t* data) {
  string known;
  switch (data->param) {
    case MIDI_PARAM_MOD_WHEEL:
      known = "(mod wheel)";
      break;
    case MIDI_PARAM_CHANNEL_VOLUME:
      known = "(chan vol)";
      break;
    case MIDI_PARAM_SUSTAIN:
      known = "(sustain ped)";
      break;
    case MIDI_PARAM_RELEASE_TIME:
      known = "(release time)";
      break;
    case MIDI_PARAM_ATTACK_TIME:
      known = "(attack time)";
      break;
    case MIDI_PARAM_BRIGHTNESS:
      known = "(brightness)";
      break;
    case MIDI_PARAM_DECAY_TIME:
      known = "(decay time)";
  }

  ostringstream s;
  s << "ch: " << int(data->channel) << " "
    << "param: " << int(data->param) << " "
    << "value: " << int(data->value) << " "
    << known << endl;
 
  return s.str();
}

void MidiTapElement::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  midi
    | Rx::subscribe<const snd_seq_event_t*>([](const snd_seq_event_t* event) {
        switch (event->type) {
          case SND_SEQ_EVENT_NOTEON:
            cout << "NOTEON " << dumpNote((snd_seq_ev_note_t*)(&event->data));
            break;
          case SND_SEQ_EVENT_NOTEOFF:
            cout << "NOTEOFF " << dumpNote((snd_seq_ev_note_t*)(&event->data));
            break;
          case SND_SEQ_EVENT_NOTE:
            cout << "NOTE " << dumpNote((snd_seq_ev_note_t*)(&event->data));
            break;
          case SND_SEQ_EVENT_KEYPRESS:
            cout << "KEY PRESSURE " << dumpNote((snd_seq_ev_note_t*)(&event->data));
            break;
          case SND_SEQ_EVENT_CONTROLLER:
            cout << "CONTROLLER " << dumpControl((snd_seq_ev_ctrl_t*)(&event->data));
            break;
          case SND_SEQ_EVENT_PGMCHANGE:
            cout << "PGMCHANGE " << dumpControl((snd_seq_ev_ctrl_t*)(&event->data));
            break;
          case SND_SEQ_EVENT_PITCHBEND:
            cout << "PITCH BEND " << dumpControl((snd_seq_ev_ctrl_t*)(&event->data));
            break;
          case SND_SEQ_EVENT_CHANPRESS:
            cout << "CHAN PRESSURE " << dumpControl((snd_seq_ev_ctrl_t*)(&event->data));
            break;
        }
      });
}

MidiTapElement::~MidiTapElement() {
  sub.unsubscribe();
}
