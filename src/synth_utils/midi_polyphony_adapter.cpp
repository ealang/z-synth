#include "./midi_polyphony_adapter.h"
#include "../synth_utils/midi_filters.h"

using namespace std;

MidiPolyphonyAdapter::MidiPolyphonyAdapter(
  uint32_t polyphony
): polyphony(polyphony) {

  for (uint32_t i = 0; i < polyphony; ++i)
  {
    Rx::subject<const snd_seq_event_t*> subject;

    subjects.emplace_back(subject);
    subject_subscribers.emplace_back(subject.get_subscriber());

    lru_voices.emplace_back(i);
    voice_to_note.emplace_back(0);
  }
}

MidiPolyphonyAdapter::~MidiPolyphonyAdapter() {
  for (auto sub: subs) {
    sub.unsubscribe();
  }
}

void MidiPolyphonyAdapter::noteOnEvent(const snd_seq_event_t* event)
{
  unsigned char note = event->data.note.note;
  uint32_t voice_id = lru_voices.back();
  lru_voices.pop_back();
  lru_voices.insert(lru_voices.begin(), voice_id);

  auto cur_note_id = voice_to_note[voice_id];
  if (cur_note_id != 0)
  {
    note_to_voice.erase(cur_note_id);
  }

  voice_to_note[voice_id] = note;
  note_to_voice[note] = voice_id;

  subject_subscribers[voice_id].on_next(event);
}

void MidiPolyphonyAdapter::noteOffEvent(const snd_seq_event_t* event)
{
  unsigned char note = event->data.note.note;
  const auto it = note_to_voice.find(note);
  if (it != note_to_voice.cend())
  {
    auto voice_id = it->second;
    note_to_voice.erase(it);

    voice_to_note[voice_id] = 0;

    subject_subscribers[voice_id].on_next(event);
  }
}

void MidiPolyphonyAdapter::injectMidi(Rx::observable<const snd_seq_event_t*> midi)
{
  NoteListener::injectMidi(midi);
  auto notNotes = midi | Rx::filter([](const snd_seq_event_t* event) {
      return !noteFilter(event);
  });

  for (const auto &subscriber: subject_subscribers)
  {
    subs.emplace_back(
      notNotes | Rx::subscribe<const snd_seq_event_t*>(subscriber)
    );
  }
}

Rx::observable<const snd_seq_event_t*> MidiPolyphonyAdapter::voiceChannel(uint32_t voiceNumber) const
{
  return subjects[voiceNumber].get_observable();
}

uint32_t MidiPolyphonyAdapter::polyphonyCount() const
{
  return polyphony;
}
