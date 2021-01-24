#include "./replica_synth.h"

#include "./elements/amp_element.h"
#include "./elements/distortion_element.h"
#include "./elements/generator_element.h"
#include "./synth_utils/generator_functions.h"
#include "./pipeline/pipeline_builder.h"

using namespace std;

shared_ptr<AudioElement<float>> ReplicaSynth::makeWiring() const {
  PipelineBuilder<float> builder;

  for (uint32_t i = 0; i < polyphonyCount; ++i) {
    char name[20];
    snprintf(name, sizeof(name), "square%d", i);
    builder.registerElem(name, genElements[i]);
    builder.connectElems(name, "amp");
  }

  builder.registerElem("amp", ampElement);
  builder.setOutputElem("amp");

  return builder.build(params.bufferSampleCount);
}

ReplicaSynth::ReplicaSynth(AudioParams params)
  : params(params),
    polyphonyPartitioning(polyphonyCount),
    ampElement(make_shared<AmpElement>(0.1)),
    distElement(make_shared<DistortionElement>())
{
  for (uint32_t i = 0; i < polyphonyCount; ++i) {
    auto squareElem = make_shared<GeneratorElement>(params.sampleRateHz, sine_function);
    genElements.emplace_back(squareElem);
  }

  _pipeline = makeWiring();
}

std::shared_ptr<AudioElement<float>> ReplicaSynth::pipeline() const
{
  return _pipeline;
}

void ReplicaSynth::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  MidiNoteListener::injectMidi(midi);
  MidiNRPNListener::injectMidi(midi);
}


void ReplicaSynth::onNoteOnEvent(unsigned char note, unsigned char velocity) {
  uint32_t voice = polyphonyPartitioning.onNoteOnEvent(note);
  genElements[voice]->onNoteOnEvent(note, velocity);
}

void ReplicaSynth::onNoteOffEvent(unsigned char note) {
  uint32_t voice = polyphonyPartitioning.onNoteOffEvent(note);
  if (voice < polyphonyCount) {
    genElements[voice]->onNoteOffEvent(note);
  }
}

void ReplicaSynth::onSustainOnEvent() {
  for(const auto &elem: genElements) {
    elem->onSustainOnEvent();
  }
}

void ReplicaSynth::onSustainOffEvent() {
  for(const auto &elem: genElements) {
    elem->onSustainOffEvent();
  }
}

void ReplicaSynth::onNRPNValueHighChange(
  unsigned char paramHigh,
  unsigned char paramLow,
  unsigned char paramValue
) {
  if (paramHigh == 0x13 && paramLow == 0x37) {
    function<float(uint32_t, uint32_t)> value;
    if (paramValue == 0) {
      value = square_function;
    } else {
      value = sine_function;
    }
    for (auto &elem: genElements) {
      elem->replaceValue(value);
    }
  }
}
