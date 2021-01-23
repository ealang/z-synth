#ifndef MIDI_NRPN_LISTENER
#define MIDI_NRPN_LISTENER

#include "../rx_include.h"
#include <alsa/seq_event.h>

#include <vector>

#define MIDI_NRPN_NULL 127

class MidiNRPNListener {
  std::vector<Rx::subscription> subs;

  unsigned char activeParamHigh = MIDI_NRPN_NULL;
  unsigned char activeParamLow = MIDI_NRPN_NULL;

public:
  virtual ~MidiNRPNListener();

protected:
  void injectMidi(Rx::observable<const snd_seq_event_t*>);

  virtual void onNRPNValueHighChange(
    unsigned char paramHigh,
    unsigned char paramLow,
    unsigned char value
  );
  virtual void onNRPNValueLowChange(
    unsigned char paramHigh,
    unsigned char paramLow,
    unsigned char value
  );
};

#endif
