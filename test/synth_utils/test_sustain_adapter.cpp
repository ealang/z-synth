#include <gtest/gtest.h>
#include <vector>

#include "../../src/synth_utils/sustain_adapter.h"

using namespace std;

struct NoteOn {
  unsigned char note;
  unsigned char velocity;
  bool operator==(const NoteOn& other) const {
    return other.note == note && other.velocity == velocity;
  }
};

class Impl: public SustainAdapter {
public:
  vector<NoteOn> ons;
  vector<unsigned char> offs;

  void noteOnEvent(unsigned char note, unsigned char vel) {
    SustainAdapter::noteOnEvent(note, vel);
  }

  void noteOffEvent(unsigned char note) {
    SustainAdapter::noteOffEvent(note);
  }

  void sustainOnEvent() {
    SustainAdapter::sustainOnEvent();
  }

  void sustainOffEvent() {
    SustainAdapter::sustainOffEvent();
  }

private:
  void sustainNoteOnEvent(unsigned char note, unsigned char vel) {
    ons.push_back(NoteOn { note, vel });
  }
  void sustainNoteOffEvent(unsigned char note) {
    offs.push_back(note);
  }
};

TEST(SustainAdapterTest, beforeAnyPresses_NoEventsAreEmitted) {
  auto i = Impl();
  ASSERT_EQ(i.ons.size(), 0);
  ASSERT_EQ(i.offs.size(), 0);
}

TEST(SustainAdapterTest, givenPressesWithNoPedal_ItPassesThePressesThrough) {
  auto i = Impl();
  vector<NoteOn> expectedOns;
  vector<unsigned char> expectedOffs;

  i.noteOnEvent(40, 127);

  expectedOns.push_back({40, 127});
  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);

  i.noteOnEvent(41, 64);

  expectedOns.push_back({41, 64});
  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);

  i.noteOffEvent(40);

  expectedOffs.push_back({40});
  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);
}

TEST(SustainAdapterTest, givenPressesThenPedal_ItDelaysOffEventsUntilPedalAndPressesAreLifted) {
  auto i = Impl();
  vector<NoteOn> expectedOns;
  vector<unsigned char> expectedOffs;

  i.noteOnEvent(40, 127);
  i.noteOnEvent(41, 127);

  expectedOns.push_back({40, 127});
  expectedOns.push_back({41, 127});
  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);

  i.sustainOnEvent();
  i.noteOffEvent(40);

  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);

  i.sustainOffEvent();

  expectedOffs.push_back({40});
  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);

  i.noteOffEvent(41);

  expectedOffs.push_back({41});
  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);
}

TEST(SustainAdapterTest, givenPedalThenPresses_ItCapturesAndSustainsPresses) {
  auto i = Impl();
  vector<NoteOn> expectedOns;
  vector<unsigned char> expectedOffs;

  i.sustainOnEvent();

  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);

  i.noteOnEvent(40, 127);

  expectedOns.push_back({40, 127});
  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);

  i.noteOffEvent(40);

  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);

  i.sustainOffEvent();

  expectedOffs.push_back(40);
  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);
}

TEST(SustainAdapterTest, givenPressesOverAlreadySustainedPresses_ItReplaysTheNote) {
  auto i = Impl();
  vector<NoteOn> expectedOns;
  vector<unsigned char> expectedOffs;

  i.noteOnEvent(40, 64);
  i.sustainOnEvent();

  expectedOns.push_back({40, 64});
  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);

  i.noteOffEvent(40);

  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);

  // note at lower velocity
  i.noteOnEvent(40, 60);

  // replayed note at original velocity
  expectedOffs.push_back({40});
  expectedOns.push_back({40, 64});
  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);

  // note at higher velocity
  i.noteOnEvent(40, 70);

  // replayed note at higher velocity
  expectedOffs.push_back({40});
  expectedOns.push_back({40, 70});
  ASSERT_EQ(i.ons, expectedOns);
  ASSERT_EQ(i.offs, expectedOffs);
}
