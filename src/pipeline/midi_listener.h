#ifndef MIDI_LISTENER_H
#define MIDI_LISTENER_H

#include <cstdint>

class MidiListener {
public:
  virtual void noteOnEvent(uint8_t, uint8_t) {}
  virtual void noteOffEvent(uint8_t) {}
  virtual void sustainOnEvent() {}
  virtual void sustainOffEvent() {}
  virtual void channelPressureEvent(uint8_t) {}
};

#endif
