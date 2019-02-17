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

void GeneratorElement::noteOnEvent(unsigned char note, unsigned char vel) {
  if (heldNotes.count(note) == 0) {
    int myId = nextId++;
    auto synth = make_shared<NoteSynth>(
        sampleRateHz,
        channelCount,
        midiNoteToFreq(note),
        vel / 127.f
    );
    heldNotes.insert(note);
    if (sustained) {
      sustainedNotes.insert(note);
    }
    if (noteSynths.count(note) > 0) {
      synths[noteSynths[note]]->postOffEvent();
      noteSynths.erase(note);
    }
    noteSynths.emplace(make_pair(note, myId));
    synths.emplace(make_pair(myId, synth));
  }
}

void GeneratorElement::noteOffEvent(unsigned char note) {
  if (heldNotes.count(note) > 0) {
    heldNotes.erase(note);
    if (!sustained) {
      synths[noteSynths[note]]->postOffEvent();
      noteSynths.erase(note);
    }
  }
}

void GeneratorElement::sustainOnEvent() {
  sustained = true;
  for (auto note: heldNotes) {
    sustainedNotes.insert(note);
  }
}

void GeneratorElement::sustainOffEvent() {
  sustained = false;
  for (auto note: sustainedNotes) {
    if (heldNotes.count(note) == 0) {
      synths[noteSynths[note]]->postOffEvent();
      noteSynths.erase(note);
    }
  }
  sustainedNotes.clear();
}

uint32_t GeneratorElement::maxInputs() {
  return 0;
}

void GeneratorElement::generate(uint32_t numSamples, float* out, uint32_t, inputs_t<float>) {
  auto dead = unordered_set<int>();

  memset(out, 0, sizeof(float) * numSamples * channelCount);

  for (auto& kv: synths) {
    auto synth = kv.second;
    if (synth->isExhausted()) {
      dead.insert(kv.first);
    } else {
      synth->generate(numSamples, out);
    }
  }

  for (auto id: dead) {
    synths.erase(id);
  }
}
