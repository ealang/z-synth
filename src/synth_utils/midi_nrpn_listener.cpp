#include "./midi_nrpn_listener.h"
#include "./midi_filters.h"

MidiNRPNListener::~MidiNRPNListener() {
  for (auto &sub: subs) {
    sub.unsubscribe();
  }
}

void MidiNRPNListener::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  subs.emplace_back(midi
    | Rx::filter(controlFilter(MIDI_PARAM_NRPN_MSB))
    | Rx::map(controlMap)
    | Rx::subscribe<int>([this](int value) {
      activeParamHigh = value;
  }));

  subs.emplace_back(midi
    | Rx::filter(controlFilter(MIDI_PARAM_NRPN_LSB))
    | Rx::map(controlMap)
    | Rx::subscribe<int>([this](int value) {
      activeParamLow = value;
  }));

  subs.emplace_back(midi
    | Rx::filter(controlFilter(MIDI_PARAM_DATA_MSB))
    | Rx::map(controlMap)
    | Rx::subscribe<int>([this](int value) {
      if (activeParamHigh != MIDI_NRPN_NULL && activeParamLow != MIDI_NRPN_NULL) {
        onNRPNValueHighChange(activeParamHigh, activeParamLow, value);
      }
  }));

  subs.emplace_back(midi
    | Rx::filter(controlFilter(MIDI_PARAM_DATA_LSB))
    | Rx::map(controlMap)
    | Rx::subscribe<int>([this](int value) {
      if (activeParamHigh != MIDI_NRPN_NULL && activeParamLow != MIDI_NRPN_NULL) {
        onNRPNValueLowChange(activeParamHigh, activeParamLow, value);
      }
  }));
}

void MidiNRPNListener::onNRPNValueHighChange(
  unsigned char,
  unsigned char,
  unsigned char 
) {
  // default no-op handler
}

void MidiNRPNListener::onNRPNValueLowChange(
  unsigned char,
  unsigned char,
  unsigned char
) {
  // default no-op handler
}
