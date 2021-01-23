#include "./replica_synth.h"

#include "./elements/amp_element.h"
#include "./elements/distortion_element.h"
#include "./elements/square_element.h"
#include "./pipeline/pipeline_builder.h"
#include "./synth_utils/midi_polyphony_adapter.h"

using namespace std;

shared_ptr<AudioElement<float>> ReplicaSynth::makeWiring1() const {
  PipelineBuilder<float> builder;

  for (uint32_t i = 0; i < polyphony->polyphonyCount(); ++i) {
    char name[20];
    snprintf(name, sizeof(name), "square%d", i);
    builder.registerElem(name, squareElems[i]);
    builder.connectElems(name, "amp");
  }

  builder.registerElem("amp", ampElement);
  builder.setOutputElem("amp");

  return builder.build(params.bufferSampleCount, params.channelCount);
}

shared_ptr<AudioElement<float>> ReplicaSynth::makeWiring2() const {
  PipelineBuilder<float> builder;

  for (uint32_t i = 0; i < polyphony->polyphonyCount(); ++i) {
    char name[20];
    snprintf(name, sizeof(name), "square%d", i);
    builder.registerElem(name, squareElems[i]);
    builder.connectElems(name, "amp");
  }

  builder.registerElem("amp", ampElement);
  builder.registerElem("dist", distElement);

  builder.connectElems("amp", "dist");
  builder.setOutputElem("dist");

  return builder.build(params.bufferSampleCount, params.channelCount);
}

ReplicaSynth::ReplicaSynth(AudioParams params)
  : params(params),
    polyphony(make_shared<MidiPolyphonyAdapter>(polyphonyCount)),
    ampElement(make_shared<AmpElement>(0.1, params.channelCount)),
    distElement(make_shared<DistortionElement>(params))
{
  for (uint32_t i = 0; i < polyphony->polyphonyCount(); ++i) {
    auto squareElem = make_shared<SquareElement>(params.sampleRateHz, params.channelCount);
    squareElem->injectMidi(polyphony->voiceChannel(i));
    squareElems.emplace_back(squareElem);
  }

  _pipeline = makeWiring1();
}

std::shared_ptr<AudioElement<float>> ReplicaSynth::pipeline() const
{
  return _pipeline;
}

void ReplicaSynth::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  polyphony->injectMidi(midi);
}
