#include <string.h>
#include <cassert>
#include <vector>
#include <math.h>

#include "./square_element.h"
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
  return _nextId++;
}

uint32_t GeneratorElement::maxInputs() {
  return 0;
}

void GeneratorElement::generate(uint32_t numSamples, float* out, uint32_t, inputs_t<float>) {
  auto dead = vector<int>();

  memset(out, 0, sizeof(float) * numSamples * channelCount);
  uint32_t numInputs = 1;
  const float* inputs[1] = {out};

  for (const auto& kv: synths) {
    const auto& synth = kv.second;
    if (synth->isExhausted()) {
      dead.push_back(kv.first);
    } else {
      synth->generate(numSamples, out, numInputs, inputs);
    }
  }

  for (const auto& id: dead) {
    synths.erase(id);
  }
}

void GeneratorElement::sustainNoteOnEvent(unsigned char note, unsigned char velocity) {
  if (noteSynths.count(note) > 0) {
    synths[noteSynths[note]]->postOffEvent();
  }

  auto myId = createId();
  noteSynths[note] = myId;
  synths.emplace(myId, make_shared<SquareElement>(
      sampleRateHz,
      channelCount,
      midiNoteToFreq(note),
      velocity / 127.f
  ));
}

void GeneratorElement::sustainNoteOffEvent(unsigned char note) {
  if (noteSynths.count(note) > 0) {
    synths[noteSynths[note]]->postOffEvent();
    noteSynths.erase(note);
  }
}
