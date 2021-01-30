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

TEST_F(PolyphonyPartitioningTest, givenInstance_ItProvidesPolyphonyCount) {
  ASSERT_EQ(2, polyphony.voiceCount());
}

TEST_F(PolyphonyPartitioningTest, givenSingleNote_ItAssignsToOneVoice) {
  ASSERT_EQ(1, polyphony.onNoteOnEvent(0));
  ASSERT_EQ(1, polyphony.onNoteOffEvent(0));
}

TEST_F(PolyphonyPartitioningTest, givenUnmatchedNote_ItReturnsUnassigned) {
  ASSERT_EQ(-1, polyphony.onNoteOffEvent(0));
}

TEST_F(PolyphonyPartitioningTest, givenMultipleNotes_ItAssignsToMultipleVoices) {
  unsigned char note1 = 1;
  unsigned char note2 = 2;

  ASSERT_EQ(1, polyphony.onNoteOnEvent(note1));
  ASSERT_EQ(0, polyphony.onNoteOnEvent(note2));
  ASSERT_EQ(1, polyphony.onNoteOffEvent(note1));
  ASSERT_EQ(0, polyphony.onNoteOffEvent(note2));
}

TEST_F(PolyphonyPartitioningTest, givenNoteOverload_ItReassignsVoice) {
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

TEST_F(PolyphonyPartitioningTest, givenDuplicateNoteOn_ItReturnsSameVoice) {
  unsigned char note1 = 1;
  ASSERT_EQ(1, polyphony.onNoteOnEvent(note1));
  ASSERT_EQ(1, polyphony.onNoteOnEvent(note1));

  ASSERT_EQ(1, polyphony.onNoteOffEvent(note1));
  ASSERT_EQ(-1, polyphony.onNoteOffEvent(note1));
}

TEST_F(PolyphonyPartitioningTest, givenSustainedNotes_ItDelaysRelease) {
  unsigned char note1 = 1;
  unsigned char note2 = 2;
  ASSERT_EQ(1, polyphony.onNoteOnEvent(note1));
  polyphony.onSustainOnEvent();
  ASSERT_EQ(0, polyphony.onNoteOnEvent(note2));

  // Sustain still on for voice 1
  ASSERT_EQ(-1, polyphony.onNoteOffEvent(note1));

  // Voice 1 now released
  auto voices = polyphony.onSustainOffEvent();
  ASSERT_EQ(1, voices.size());
  ASSERT_EQ(1, voices[0]);

  // Voice 0 now released
  ASSERT_EQ(0, polyphony.onNoteOffEvent(note2));
}
