#include "./pipeline_setup.h"
#include "./pipeline/pipeline_builder.h"

#include "./elements/generator_element.h"
#include "./elements/amp_element.h"
#include "./elements/midi_tap_element.h"

using namespace std;

shared_ptr<MidiAudioElement<float>> build_pipeline(AudioParams params, bool dumpMidi) {
  PipelineBuilder<float> builder;

  auto synth = make_shared<GeneratorElement>(params.sampleRateHz, params.channelCount);
  auto amp = make_shared<AmpElement>(0.04, params.channelCount);
  if (dumpMidi) {
    auto tap = make_shared<MidiTapElement>();
    builder.registerMidi(tap);
  }

  builder.registerMidi(synth);
  builder.registerMidi(amp);

  builder.registerElem("synth", synth);
  builder.registerElem("amp", amp);

  builder.connectElems("synth", "amp");
  builder.setOutputElem("amp");

  return builder.build(params.bufferSampleCount, params.channelCount);
}
