#include <gtest/gtest.h>
#include <vector>

#include "../../src/synth_utils/build_midi_event.h"
#include "../../src/synth_utils/midi_polyphony_adapter.h"

using namespace std;

class MidiPolyphonyAdapterTest: public testing::Test {
public: 
  Rx::subscription voice0Sub, voice1Sub;
  MidiPolyphonyAdapter polyphony;

  Rx::subject<const snd_seq_event_t*> subject;

  vector<const snd_seq_event_t*> voice0Events;
  vector<const snd_seq_event_t*> voice1Events;

  MidiPolyphonyAdapterTest()
    : polyphony(2) {}

  void SetUp() {
    polyphony.injectMidi(subject.get_observable());

    voice0Sub = polyphony.voiceChannel(0)
      | Rx::subscribe<const snd_seq_event_t*>([this](const snd_seq_event_t* event){
          voice0Events.emplace_back(event);
        });
    voice1Sub = polyphony.voiceChannel(1)
      | Rx::subscribe<const snd_seq_event_t*>([this](const snd_seq_event_t* event){
          voice1Events.emplace_back(event);
        });
  }

  void send(const snd_seq_event_t *event)
  {
    subject.get_subscriber().on_next(event);
  }

  void TearDown() { 
    voice0Sub.unsubscribe();
    voice1Sub.unsubscribe();
  }
};

TEST_F(MidiPolyphonyAdapterTest, provides_polyphony_count) {

  ASSERT_EQ(2, polyphony.polyphonyCount());
}

TEST_F(MidiPolyphonyAdapterTest, sends_note_on_off_pair_to_single_voice) {
  auto on_event = buildNoteOnEvent(1, 127);
  auto off_event = buildNoteOffEvent(1);

  send(&on_event);
  send(&off_event);

  ASSERT_EQ(0, voice0Events.size());
  ASSERT_EQ(2, voice1Events.size());

  ASSERT_EQ(&on_event, voice1Events[0]);
  ASSERT_EQ(&off_event, voice1Events[1]);
}

TEST_F(MidiPolyphonyAdapterTest, forwards_control_messages_to_all) {
  auto sustain = buildSustainEvent(0);

  send(&sustain);

  ASSERT_EQ(1, voice0Events.size());
  ASSERT_EQ(1, voice1Events.size());

  ASSERT_EQ(&sustain, voice0Events[0]);
  ASSERT_EQ(&sustain, voice1Events[0]);
}

TEST_F(MidiPolyphonyAdapterTest, distributes_concurrent_notes_across_voices) {
  auto on_event_1 = buildNoteOnEvent(1, 127);
  auto off_event_1 = buildNoteOffEvent(1);
  auto on_event_2 = buildNoteOnEvent(2, 127);
  auto off_event_2 = buildNoteOffEvent(2);

  send(&on_event_1);
  send(&on_event_2);
  send(&off_event_1);
  send(&off_event_2);

  ASSERT_EQ(2, voice0Events.size());
  ASSERT_EQ(2, voice1Events.size());

  ASSERT_EQ(&on_event_1, voice1Events[0]);
  ASSERT_EQ(&off_event_1, voice1Events[1]);
  ASSERT_EQ(&on_event_2, voice0Events[0]);
  ASSERT_EQ(&off_event_2, voice0Events[1]);
}

TEST_F(MidiPolyphonyAdapterTest, reuses_voices_when_polyphony_depleted) {
  // note on
  auto on_event_1 = buildNoteOnEvent(1, 127);
  auto on_event_2 = buildNoteOnEvent(2, 127);
  auto on_event_3 = buildNoteOnEvent(3, 127);

  send(&on_event_1);
  send(&on_event_2);
  send(&on_event_3);

  ASSERT_EQ(1, voice0Events.size());
  ASSERT_EQ(2, voice1Events.size());

  ASSERT_EQ(&on_event_1, voice1Events[0]);
  ASSERT_EQ(&on_event_2, voice0Events[0]);
  ASSERT_EQ(&on_event_3, voice1Events[1]);

  // note off
  auto off_event_1 = buildNoteOffEvent(1);
  auto off_event_2 = buildNoteOffEvent(2);
  auto off_event_3 = buildNoteOffEvent(3);

  send(&off_event_1);
  send(&off_event_2);
  send(&off_event_3);

  ASSERT_EQ(1 + 1, voice0Events.size());
  ASSERT_EQ(2 + 1, voice1Events.size());

  ASSERT_EQ(&off_event_2, voice0Events[0 + 1]);
  ASSERT_EQ(&off_event_3, voice1Events[1 + 1]);
}
