#include "./pipeline/pipeline_builder.h"
#include "./pipeline_setup.h"

#include "./elements/generator_element.h"
#include "./elements/amp_element.h"

using namespace std;

shared_ptr<MidiAudioElement<float>> build_pipeline(uint32_t sampleRateHz, uint32_t bufferSize, uint32_t channelCount) {
  PipelineBuilder<float> builder;

  auto synth = make_shared<GeneratorElement>(sampleRateHz, channelCount);
  auto amp = make_shared<AmpElement>(0.04, channelCount);

  builder.registerMidi(synth);
  builder.registerElem("synth", synth);
  builder.registerElem("amp", amp);

  builder.connectElems("synth", "amp");
  builder.setOutputElem("amp");

  return builder.build(bufferSize, channelCount);
}
