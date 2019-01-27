#include <math.h>
#include <cstring>
#include "midi_mux.h"

using namespace std;

static float midiNoteToFreq(unsigned char note) {
  return 440 * powf(2, (static_cast<float>(note) - 69) / 12);
}

MidiMux::MidiMux(uint32_t sampleRateHz):
  sampleRateHz(sampleRateHz) {
}

void MidiMux::noteOnEvent(unsigned char note, unsigned char vel) {
  lock_guard<mutex> guard(this->lock);

  if (notesMap.count(note) == 0) {
    auto myId = nextId++;
    auto f = midiNoteToFreq(note);
    auto s = make_shared<NoteSynth>(this->sampleRateHz, f, vel / 127.f);
    printf("adding client %d with freq %f\n", (int)myId, f);
    synths.emplace(make_pair(myId, s));
    notesMap[note] = myId;
  }
}

void MidiMux::noteOffEvent(unsigned char note) {
  lock_guard<mutex> guard(this->lock);

  if (notesMap.count(note) > 0) {
    synths[notesMap[note]].get()->postOffEvent();
    notesMap.erase(note);
  }
}

void MidiMux::channelPressureEvent(unsigned char pressure) {
  lock_guard<mutex> guard(this->lock);

  for (auto& kv: this->synths) {
    kv.second.get()->postPressureEvent(pressure);
  }
}

void MidiMux::generate(sample_t* buffer, int count) {
  lock_guard<mutex> guard(this->lock);

  auto dead = unordered_set<int>();
  memset(buffer, 0, count * sizeof(sample_t));
  for (auto& kv: this->synths) {
    NoteSynth *synth = kv.second.get();
    if (synth->isExhausted()) {
      dead.insert(kv.first);
    } else {
      synth->generate(count, buffer);
    }
  }
  for (auto id: dead) {
    printf("erasing client %d\n", id);
    this->synths.erase(id);
  }
}
