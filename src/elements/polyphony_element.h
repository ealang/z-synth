#ifndef POLYPHONY_ELEMENT_H
#define POLYPHONY_ELEMENT_H

#include <cstdint> 
#include <vector>

#include "../synth_utils/sustain_adapter.h"

// Create a per-voice midi event stream.
class PolyphonyElement: public SustainAdapter {
  uint32_t polyphony;
  std::vector<Rx::subscription> subs;
  std::vector<Rx::subject<const snd_seq_event_t*>> subjects;
  std::vector<Rx::subscriber<const snd_seq_event_t*>> subject_subscribers;

  std::unordered_map<unsigned char, uint32_t> note_to_voice;
  std::vector<uint32_t> voice_to_note;
  std::vector<uint32_t> lru_voices;

  void sustainNoteOnEvent(unsigned char note, unsigned char vel) override;
  void sustainNoteOffEvent(unsigned char note) override;

public:
  PolyphonyElement(uint32_t polyphony);
  ~PolyphonyElement();

  void injectMidi(Rx::observable<const snd_seq_event_t*>) override;
  Rx::observable<const snd_seq_event_t*> voiceChannel(uint32_t voiceNumber) const;
  uint32_t polyphonyCount() const;
};

#endif
