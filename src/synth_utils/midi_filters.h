#ifndef MIDI_FILTERS_H
#define MIDI_FILTERS_H

#include <alsa/seq_event.h>
#include <functional>
#include <cstdint>
#include <tuple>

std::function<bool(const snd_seq_event_t*)> channelFilter(unsigned char channel);

bool noteFilter(const snd_seq_event_t* event);
std::tuple<bool, uint8_t, uint8_t> noteMap(const snd_seq_event_t* event);

// filter control messages matching `param`
std::function<bool(const snd_seq_event_t*)> controlFilter(uint32_t param);
// return value of the control message
int controlMap(const snd_seq_event_t* event);

// Control change message numbers
#define MIDI_PARAM_MOD_WHEEL 1
#define MIDI_PARAM_CHANNEL_VOLUME 7
#define MIDI_PARAM_SUSTAIN 64
#define MIDI_PARAM_RELEASE_TIME 72
#define MIDI_PARAM_ATTACK_TIME 73
#define MIDI_PARAM_BRIGHTNESS 74
#define MIDI_PARAM_DECAY_TIME 75
#define MIDI_PARAM_NRPN_MSB 99
#define MIDI_PARAM_NRPN_LSB 98
#define MIDI_PARAM_DATA_MSB 6
#define MIDI_PARAM_DATA_LSB 38

#endif
