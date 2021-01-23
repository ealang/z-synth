#ifndef POLYPHONY_ADAPTER_H
#define POLYPHONY_ADAPTER_H

#include <cstdint> 
#include <vector>
#include <unordered_map>

#include "./note_listener.h"

// Expose a per-voice midi event stream.
class MidiPolyphonyAdapter: public NoteListener {
  uint32_t polyphony;
  std::vector<Rx::subscription> subs;
  std::vector<Rx::subject<const snd_seq_event_t*>> subjects;
  std::vector<Rx::subscriber<const snd_seq_event_t*>> subject_subscribers;

  std::unordered_map<unsigned char, uint32_t> note_to_voice;
  std::vector<uint32_t> voice_to_note;
  std::vector<uint32_t> lru_voices;

  void noteOnEvent(const snd_seq_event_t*) override;
  void noteOffEvent(const snd_seq_event_t*) override;

public:
  MidiPolyphonyAdapter(uint32_t polyphony);
  ~MidiPolyphonyAdapter();

  void injectMidi(Rx::observable<const snd_seq_event_t*>);
  Rx::observable<const snd_seq_event_t*> voiceChannel(uint32_t voiceNumber) const;
  uint32_t polyphonyCount() const;
};

#endif
