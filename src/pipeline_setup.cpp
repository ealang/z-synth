#include "./pipeline/pipeline_builder.h"
#include "./pipeline_setup.h"

#include "./elements/filter_element.h"
#include "./elements/generator_element.h"
#include "./elements/amp_element.h"

using namespace std;

shared_ptr<MidiAudioElement<float>> build_pipeline(uint32_t sampleRateHz, uint32_t bufferSize, uint32_t channelCount) {
  PipelineBuilder<float> builder;

  shared_ptr<MidiAudioElement<float>> synth = make_shared<GeneratorElement>(sampleRateHz, channelCount);
  shared_ptr<AudioElement<float>> filter = make_shared<FilterElement>(0.0002, sampleRateHz, channelCount);
  shared_ptr<AudioElement<float>> amp = make_shared<AmpElement>(0.04, channelCount);

  builder.registerElem("synth", synth);
  builder.registerElem("filter", filter);
  builder.registerElem("amp", amp);

  builder.connectElems("synth", "filter");
  builder.connectElems("filter", "amp");

  builder.setOutputElem("amp");

  return builder.build(bufferSize, channelCount);
}
