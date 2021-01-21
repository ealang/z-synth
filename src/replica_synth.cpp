#include "./replica_synth.h"

#include "./elements/amp_element.h"
#include "./elements/distortion_element.h"
#include "./elements/switch_element.h"
#include "./elements/square_element.h"
#include "./pipeline/pipeline_builder.h"
#include "./synth_utils/midi_polyphony_adapter.h"

using namespace std;

// Example core- just a square wave
shared_ptr<AudioElement<float>> makeSquareCore(
  const AudioParams &params,
  Rx::observable<const snd_seq_event_t*> midi
) {
  auto squareElem = make_shared<SquareElement>(params.sampleRateHz, params.channelCount);
  squareElem->injectMidi(midi);
  return squareElem;
}

// Example core- embedded pipeline
shared_ptr<AudioElement<float>> makeDistortionCore(
  const AudioParams &params,
  Rx::observable<const snd_seq_event_t*> midi
) {
  PipelineBuilder<float> builder;

  auto squareElem = make_shared<SquareElement>(params.sampleRateHz, params.channelCount);
  squareElem->injectMidi(midi);

  auto distElem = make_shared<DistortionElement>(params);
  distElem->injectMidi(midi);

  builder.registerElem("square", squareElem);
  builder.registerElem("dist", distElem);

  builder.connectElems("square", "dist");
  builder.setOutputElem("dist");

  return builder.build(params.bufferSampleCount, params.channelCount);
}

void ReplicaSynth::initPipeline() {
  PipelineBuilder<float> builder;

  for (uint32_t i = 0; i < polyphony->polyphonyCount(); ++i) {
     auto core = makeSquareCore(params, polyphony->voiceChannel(i));
     //auto core = makeDistortionCore(params, polyphony->voiceChannel(i));

    auto switchElement = std::make_shared<SwitchElement>(params.channelCount);
    switchElement->setCore(core);

    char name[20];
    snprintf(name, sizeof(name), "switch%d", i);
    builder.registerElem(name, switchElement);
    builder.connectElems(name, "amp");

    switchElements.emplace_back(switchElement);
  }

  builder.registerElem("amp", ampElement);
  builder.setOutputElem("amp");

  _pipeline = builder.build(params.bufferSampleCount, params.channelCount);
}

ReplicaSynth::ReplicaSynth(AudioParams params)
  : params(params),
    polyphony(make_shared<MidiPolyphonyAdapter>(polyphonyCount)),
    ampElement(make_shared<AmpElement>(0.04, params.channelCount))
{
  initPipeline();
}

std::shared_ptr<AudioElement<float>> ReplicaSynth::pipeline() const
{
  return _pipeline;
}

void ReplicaSynth::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  ampElement->injectMidi(midi);
  polyphony->injectMidi(midi);
}
