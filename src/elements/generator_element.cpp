#include <string.h>
#include <vector>
#include <math.h>

#include "./square_element.h"
#include "./filter_element.h"
#include "./generator_element.h"
#include "../synth_utils/linear_scale.h"

using namespace std;

static const auto filtScale = linearScaleClamped(30, 96, 50./41000, 7./41000);
static const auto ampScale = linearScaleClamped(60, 96, 1, 0.15);

static float midiNoteToFreq(unsigned char note) {
  return 440 * powf(2, (static_cast<float>(note) - 69) / 12);
}

/* Ad hoc per-note pipeline. */
class NotePipeline: public AudioElement<float> {
  vector<float> buffer;
  uint32_t channelCount;

  SquareElement square;
  FilterElement filter;

public:
  NotePipeline(
    uint32_t sampleRateHz,
    uint32_t channelCount,
    unsigned char note,
    unsigned char velocity
  ): channelCount(channelCount),
     square(SquareElement(sampleRateHz, channelCount, midiNoteToFreq(note), (velocity / 127.) * ampScale(note))),
     filter(filtScale(note) * sampleRateHz, channelCount)
  {}

  bool isExhausted() {
    // todo: should also wait for the filter
    return square.isExhausted();
  }

  void postOffEvent() {
    return square.postOffEvent();
  }

  uint32_t maxInputs() override {
    return 0;
  }

  void generate(
    uint32_t numSamples,
    float* out,
    uint32_t,
    inputs_t<float>
  ) override {
    if (buffer.size() < numSamples * channelCount) {
      buffer.resize(numSamples * channelCount);
    }

    square.generate(numSamples, buffer.data(), 0, nullptr);

    float* filterInputs[] = { buffer.data() };
    filter.generate(numSamples, out, 1, filterInputs);
  }
};

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
  uint32_t bufferSize = numSamples * channelCount;
  if (stagingBuffer.size() < bufferSize) {
    stagingBuffer.resize(bufferSize);
  }

  memset(out, 0, sizeof(float) * bufferSize);

  auto dead = vector<uint32_t>();
  for (const auto& kv: pipelines) {
    const auto& pipeline = kv.second;
    if (pipeline->isExhausted()) {
      dead.push_back(kv.first);
    } else {
      pipeline->generate(numSamples, stagingBuffer.data(), 0, nullptr);
      for (uint32_t i = 0; i < bufferSize; i++) {
        out[i] += stagingBuffer[i];
      }
    }
  }

  for (const auto& id: dead) {
    pipelines.erase(id);
  }
}

void GeneratorElement::sustainNoteOnEvent(unsigned char note, unsigned char velocity) {
  if (noteSynths.count(note) > 0) {
    pipelines[noteSynths[note]]->postOffEvent();
  }

  auto myId = createId();
  noteSynths[note] = myId;
  pipelines.emplace(myId, make_shared<NotePipeline>(
      sampleRateHz,
      channelCount,
      note,
      velocity
  ));
}

void GeneratorElement::sustainNoteOffEvent(unsigned char note) {
  if (noteSynths.count(note) > 0) {
    pipelines[noteSynths[note]]->postOffEvent();
    noteSynths.erase(note);
  }
}
