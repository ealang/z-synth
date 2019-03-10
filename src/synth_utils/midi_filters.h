#ifndef MIDI_FILTERS_H
#define MIDI_FILTERS_H

#include <alsa/seq_event.h>
#include <functional>
#include <cstdint>
#include <tuple>

std::function<bool(const snd_seq_event_t*)> channelFilter(unsigned char channel);

bool noteFilter(const snd_seq_event_t* event);
std::tuple<bool, uint8_t, uint8_t> noteMap(const snd_seq_event_t* event);

bool sustainFilter(const snd_seq_event_t* event);
uint8_t sustainMap(const snd_seq_event_t* event);

#endif
