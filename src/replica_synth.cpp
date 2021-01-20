#include "./replica_synth.h"

#include "./elements/amp_element.h"
#include "./elements/distortion_element.h"
#include "./elements/square_element.h"
#include "./pipeline/pipeline_builder.h"
#include "./synth_utils/midi_polyphony_adapter.h"

#include <cstdio>

using namespace std;

static shared_ptr<AudioElement<float>> buildPipeline(
    AudioParams params,
    Rx::observable<const snd_seq_event_t*> globalMidi,
    shared_ptr<MidiPolyphonyAdapter> polyphony
) {
  PipelineBuilder<float> builder;

  for (uint32_t i = 0; i < polyphony->polyphonyCount(); ++i) {
    auto synth = std::make_shared<SquareElement>(params.sampleRateHz, params.channelCount);

    synth->injectMidi(polyphony->voiceChannel(i));

    char name[20];
    snprintf(name, sizeof(name), "synth%d", i);
    builder.registerElem(name, synth);
    builder.connectElems(name, "amp");
  }

  auto amp = make_shared<AmpElement>(0.04, params.channelCount);
  amp->injectMidi(globalMidi);
  builder.registerElem("amp", amp);

  builder.setOutputElem("amp");

  return builder.build(params.bufferSampleCount, params.channelCount);
}

ReplicaSynth::ReplicaSynth(AudioParams params, Rx::observable<const snd_seq_event_t*> globalMidi)
  : polyphony(make_shared<MidiPolyphonyAdapter>(polyphonyCount))
{
  polyphony->injectMidi(globalMidi);
  p = buildPipeline(params, globalMidi, polyphony);
}

std::shared_ptr<AudioElement<float>> &ReplicaSynth::pipeline()
{
  return p;
}
