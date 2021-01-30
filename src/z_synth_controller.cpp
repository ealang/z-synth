#include "./z_synth_controller.h"

#include "./elements/adsr_element.h"
#include "./elements/amp_element.h"
#include "./elements/distortion_element.h"
#include "./elements/generator_element.h"
#include "./synth_utils/generator_functions.h"
#include "./synth_utils/midi_note_to_freq.h"
#include "./pipeline/pipeline_builder.h"

using namespace std;

shared_ptr<AudioElement<float>> ZSynthController::makeWiring() const {
  PipelineBuilder<float> builder;

  builder.registerElem("lfo", lfoElement);
  builder.registerElem("dist", distElement);

  for (uint32_t i = 0; i < polyphonyCount; ++i) {
    char genName[20];
    snprintf(genName, sizeof(genName), "gen%d", i);
    builder.registerElem(genName, genElements[i]);
    builder.connectElems("lfo", genName, genElements[i]->fmPortNumber());

    char adsrName[20];
    snprintf(adsrName, sizeof(adsrName), "adsr%d", i);
    builder.registerElem(adsrName, adsrElements[i]);

    builder.connectElems(genName, adsrName, adsrElements[i]->inputPortNumber());
    builder.connectElems(adsrName, "amp", ampElement->inputPortNumber(i));
  }

  builder.registerElem("amp", ampElement);
  builder.connectElems("amp", "dist", distElement->inputPortNumber());
  builder.setOutputElem("dist");

  return builder.build(params.bufferSampleCount);
}

ZSynthController::ZSynthController(AudioParams params)
  : params(params),
    polyphonyPartitioning(polyphonyCount),
    ampElement(make_shared<AmpElement>(0.1)),
    lfoElement(make_shared<GeneratorElement>(params.sampleRateHz, sine_function)),
    distElement(make_shared<DistortionElement>(0.8))
{
  for (uint32_t i = 0; i < polyphonyCount; ++i) {
    genElements.emplace_back(
      make_shared<GeneratorElement>(params.sampleRateHz, sine_function)
    );
    adsrElements.emplace_back(
      make_shared<ADSRElement>(params.sampleRateHz)
    );
  }

  lfoElement->setFrequency(4);
  lfoElement->setEnabled(true);

  _pipeline = makeWiring();
}

std::shared_ptr<AudioElement<float>> ZSynthController::pipeline() const
{
  return _pipeline;
}

void ZSynthController::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  MidiNoteListener::injectMidi(midi);
  MidiNRPNListener::injectMidi(midi);
}


void ZSynthController::onNoteOnEvent(unsigned char note, unsigned char) {
  uint32_t voice = polyphonyPartitioning.onNoteOnEvent(note);
  genElements[voice]->setFrequency(midiNoteToFreq(note));
  genElements[voice]->setEnabled(true);
  adsrElements[voice]->trigger();
}

void ZSynthController::onNoteOffEvent(unsigned char note) {
  uint32_t voice = polyphonyPartitioning.onNoteOffEvent(note);
  if (voice < polyphonyCount) {
    adsrElements[voice]->release();
  }
}

void ZSynthController::onSustainOnEvent() {
  polyphonyPartitioning.onSustainOnEvent();
}

void ZSynthController::onSustainOffEvent() {
  for(const auto &voice: polyphonyPartitioning.onSustainOffEvent()) {
    if (voice < polyphonyCount) {
      adsrElements[voice]->release();
    }
  }
}

void ZSynthController::onNRPNValueHighChange(
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
