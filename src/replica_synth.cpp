#include "./replica_synth.h"
#include "./pipeline/pipeline_builder.h"

#include "./elements/amp_element.h"
#include "./elements/distortion_element.h"
#include "./elements/generator_element.h"
#include "./elements/polyphony_element.h"

using namespace std;

static shared_ptr<AudioElement<float>> build_pipeline(
    AudioParams params,
    Rx::observable<const snd_seq_event_t*> globalMidi,
    shared_ptr<PolyphonyElement> polyphony
) {
  PipelineBuilder<float> builder;

  shared_ptr<MidiAudioElement<float>> synth = make_shared<GeneratorElement>(params.sampleRateHz, params.channelCount);
  auto amp = make_shared<AmpElement>(0.04, params.channelCount);
  auto dist = make_shared<DistortionElement>(params);

  synth->injectMidi(polyphony->voiceEvents(0));
  amp->injectMidi(globalMidi);
  dist->injectMidi(globalMidi);

  builder.registerElem("synth", synth);
  builder.registerElem("dist", dist);
  builder.registerElem("amp", amp);

  builder.connectElems("synth", "dist");
  builder.connectElems("dist", "amp");
  builder.setOutputElem("amp");

  return builder.build(params.bufferSampleCount, params.channelCount);
}

ReplicaSynth::ReplicaSynth(AudioParams params, Rx::observable<const snd_seq_event_t*> globalMidi)
  : polyphony(make_shared<PolyphonyElement>(polyphonyCount))
{
  polyphony->injectMidi(globalMidi);
  p = build_pipeline(params, globalMidi, polyphony);
}

std::shared_ptr<AudioElement<float>> &ReplicaSynth::pipeline()
{
  return p;
}
