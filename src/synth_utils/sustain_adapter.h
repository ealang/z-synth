#ifndef SUSTAIN_ADAPTER_H
#define SUSTAIN_ADAPTER_H

#include <cstdint> 
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "../pipeline/pipeline_element.h"

/* Listen to raw note and sustain pedal events. Emit new note events
 * modified by the sustain pedal.
 */
class SustainAdapter: public MidiListener {
  // Maintain a set of held and sustained data, and bounce
  // notes back and forth based on the state of the pedal.
  std::unordered_map<unsigned char, unsigned char> heldPlays;
  std::unordered_map<unsigned char, unsigned char> sustainPlays;
  std::unordered_set<unsigned char> heldNotes;
  bool sustained = false;

public:
  void noteOnEvent(unsigned char note, unsigned char vel) override;
  void noteOffEvent(unsigned char note) override;
  void sustainOnEvent() override;
  void sustainOffEvent() override;

protected:
  // Callbacks to be implemented
  virtual void sustainNoteOnEvent(unsigned char note, unsigned char vel) = 0;
  virtual void sustainNoteOffEvent(unsigned char note) = 0;
};

#endif
