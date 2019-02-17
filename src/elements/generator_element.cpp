#include <cassert>
#include <cstring>
#include <math.h>
#include <vector>

#include "./generator_element.h"

using namespace std;

static float midiNoteToFreq(unsigned char note) {
  return 440 * powf(2, (static_cast<float>(note) - 69) / 12);
}

GeneratorElement::GeneratorElement(
  uint32_t sampleRateHz,
  uint32_t channelCount
): sampleRateHz(sampleRateHz), channelCount(channelCount) {
}

uint32_t GeneratorElement::createId() {
  return nextId++;
}

void GeneratorElement::noteOnEvent(unsigned char note, unsigned char vel) {
  uint32_t myId = createId();
  unsigned char adjustedVelocity = vel;
  heldNotes.insert(note);

  if (sustained) {
    if (sustainSynths.count(note) > 0) {
      synths[sustainSynths[note]]->postOffEvent();
      adjustedVelocity = max(adjustedVelocity, sustainVelocity[note]);
    }
    sustainSynths[note] = myId;
    sustainVelocity[note] = adjustedVelocity;
  } else {
    if (heldSynths.count(note) > 0) {
      synths[heldSynths[note]]->postOffEvent(); // in case of multiple noteOn events
    }
    heldSynths[note] = myId;
    heldVelocity[note] = vel;
  }

  synths.emplace(myId, make_shared<SquareElement>(
      sampleRateHz,
      channelCount,
      midiNoteToFreq(note),
      adjustedVelocity / 127.f
  ));
}

void GeneratorElement::noteOffEvent(unsigned char note) {
  heldNotes.erase(note);
  if (heldSynths.count(note) > 0) {
    synths[heldSynths[note]]->postOffEvent();
    heldSynths.erase(note);
    heldVelocity.erase(note);
  }
}

void GeneratorElement::sustainOnEvent() {
  // in case of multiple sustainOn events
  for (auto it: sustainSynths) {
    synths[it.second]->postOffEvent();
  }

  sustainSynths = heldSynths;
  sustainVelocity = heldVelocity;
  heldSynths.clear();
  heldVelocity.clear();

  sustained = true;
}

void GeneratorElement::sustainOffEvent() {
  auto synthIt = sustainSynths.begin();
  auto velIt = sustainVelocity.begin();
  while (synthIt != sustainSynths.end()) {
    unsigned char note = synthIt->first;
    uint32_t synth = synthIt->second;

    // transfer any still held notes from sustain to held
    if (heldNotes.count(note) == 0) {
      synths[synth]->postOffEvent();
    } else {
      heldSynths.emplace(note, synth);
      heldVelocity.emplace(note, velIt->second);
    }

    synthIt++;
    velIt++;
  }
  assert(velIt == sustainVelocity.end());

  sustainSynths.clear();
  sustainVelocity.clear();
  sustained = false;
}

uint32_t GeneratorElement::maxInputs() {
  return 0;
}

void GeneratorElement::generate(uint32_t numSamples, float* out, uint32_t, inputs_t<float>) {
  auto dead = unordered_set<int>();

  memset(out, 0, sizeof(float) * numSamples * channelCount);
  uint32_t numInputs = 1;
  const float* inputs[1] = {out};

  for (auto& kv: synths) {
    auto synth = kv.second;
    if (synth->isExhausted()) {
      dead.insert(kv.first);
    } else {
      synth->generate(numSamples, out, numInputs, inputs);
    }
  }

  for (auto id: dead) {
    synths.erase(id);
  }
}
