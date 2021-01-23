#include "../../src/synth_utils/polyphony_partitioning.h"

#include <gtest/gtest.h>
#include <vector>

using namespace std;

class PolyphonyPartitioningTest: public testing::Test {
public: 
  PolyphonyPartitioning polyphony;

  PolyphonyPartitioningTest()
    : polyphony(2) {}
};

TEST_F(PolyphonyPartitioningTest, provides_polyphony_count) {
  ASSERT_EQ(2, polyphony.voiceCount());
}

TEST_F(PolyphonyPartitioningTest, sends_note_on_off_pair_to_single_voice) {
  ASSERT_EQ(1, polyphony.onNoteOnEvent(0));
  ASSERT_EQ(1, polyphony.onNoteOffEvent(0));
}

TEST_F(PolyphonyPartitioningTest, sends_unmatched_note_off_to_unassigned) {
  ASSERT_EQ(-1, polyphony.onNoteOffEvent(0));
}

TEST_F(PolyphonyPartitioningTest, distributes_concurrent_notes_across_voices) {
  unsigned char note1 = 1;
  unsigned char note2 = 2;

  ASSERT_EQ(1, polyphony.onNoteOnEvent(note1));
  ASSERT_EQ(0, polyphony.onNoteOnEvent(note2));
  ASSERT_EQ(1, polyphony.onNoteOffEvent(note1));
  ASSERT_EQ(0, polyphony.onNoteOffEvent(note2));
}

TEST_F(PolyphonyPartitioningTest, reuses_voices_when_polyphony_depleted) {
  unsigned char note1 = 1;
  unsigned char note2 = 2;
  unsigned char note3 = 3;

  ASSERT_EQ(1, polyphony.onNoteOnEvent(note1));
  ASSERT_EQ(0, polyphony.onNoteOnEvent(note2));
  ASSERT_EQ(1, polyphony.onNoteOnEvent(note3));

  ASSERT_EQ(-1, polyphony.onNoteOffEvent(note1));
  ASSERT_EQ(0, polyphony.onNoteOffEvent(note2));
  ASSERT_EQ(1, polyphony.onNoteOffEvent(note3));
}

TEST_F(PolyphonyPartitioningTest, duplicate_note_on_assigned_to_same_voice) {
  unsigned char note1 = 1;
  ASSERT_EQ(1, polyphony.onNoteOnEvent(note1));
  ASSERT_EQ(1, polyphony.onNoteOnEvent(note1));

  ASSERT_EQ(1, polyphony.onNoteOffEvent(note1));
  ASSERT_EQ(-1, polyphony.onNoteOffEvent(note1));
}
