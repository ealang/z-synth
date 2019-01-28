#include <cstring>
#include <math.h>
#include <vector>

#include "midi_mux.h"

using namespace std;

static const float masterAmp = 0.04f;
static const uint32_t filterLen = 4;

static float midiNoteToFreq(unsigned char note) {
  return 440 * powf(2, (static_cast<float>(note) - 69) / 12);
}

MidiMux::MidiMux(
  uint32_t sampleRateHz,
  uint32_t channelCount
):
  sampleRateHz(sampleRateHz),
  channelCount(channelCount),
  filters(channelCount, Filter(filterLen)) {
}

void MidiMux::noteOnEvent(unsigned char note, unsigned char vel) {
  lock_guard<mutex> guard(lock);

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
      synths[noteSynths[note]].get()->postOffEvent();
      noteSynths.erase(note);
    }
    noteSynths.emplace(make_pair(note, myId));
    synths.emplace(make_pair(myId, synth));
  }
}

void MidiMux::noteOffEvent(unsigned char note) {
  lock_guard<mutex> guard(lock);

  if (heldNotes.count(note) > 0) {
    heldNotes.erase(note);
    if (!sustained) {
      synths[noteSynths[note]].get()->postOffEvent();
      noteSynths.erase(note);
    }
  }
}

void MidiMux::sustainOnEvent() {
  lock_guard<mutex> guard(lock);

  sustained = true;
  for (auto note: heldNotes) {
    sustainedNotes.insert(note);
  }
}

void MidiMux::sustainOffEvent() {
  lock_guard<mutex> guard(lock);

  sustained = false;
  for (auto note: sustainedNotes) {
    if (heldNotes.count(note) == 0) {
      synths[noteSynths[note]].get()->postOffEvent();
      noteSynths.erase(note);
    }
  }
  sustainedNotes.clear();
}

void MidiMux::channelPressureEvent(unsigned char pressure) {
  lock_guard<mutex> guard(lock);

  for (auto& kv: synths) {
    kv.second.get()->postPressureEvent(pressure);
  }
}

void MidiMux::generate(uint32_t nSamples, float* buffer) {
  lock_guard<mutex> guard(lock);

  auto dead = unordered_set<int>();

  for (auto& kv: synths) {
    NoteSynth *synth = kv.second.get();
    if (synth->isExhausted()) {
      dead.insert(kv.first);
    } else {
      synth->generate(nSamples, buffer);
    }
  }

  for (uint32_t i = 0; i < nSamples; i++) {
    for (uint32_t c = 0; c < channelCount; c++) {
      int j = i * channelCount + c;
      buffer[j] = filters[c].next(buffer[j]) * masterAmp;
    }
  }

  for (auto id: dead) {
    synths.erase(id);
  }
}
