#include "./z_synth_controller.h"

#include "./elements/amp_element.h"
#include "./elements/distortion_element.h"
#include "./elements/generator_element.h"
#include "./synth_utils/generator_functions.h"
#include "./pipeline/pipeline_builder.h"

using namespace std;

shared_ptr<AudioElement<float>> ZSynthController::makeWiring() const {
  PipelineBuilder<float> builder;

  builder.registerElem("lfo", lfoElement);
  builder.registerElem("dist", distElement);

  for (uint32_t i = 0; i < polyphonyCount; ++i) {
    char name[20];
    snprintf(name, sizeof(name), "square%d", i);
    builder.registerElem(name, genElements[i]);
    builder.connectElems(name, "amp", ampElement->inputPortNumber(i));
    builder.connectElems("lfo", name, genElements[i]->fmPortNumber());
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


void ZSynthController::onNoteOnEvent(unsigned char note, unsigned char velocity) {
  uint32_t voice = polyphonyPartitioning.onNoteOnEvent(note);
  genElements[voice]->onNoteOnEvent(note, velocity);
}

void ZSynthController::onNoteOffEvent(unsigned char note) {
  uint32_t voice = polyphonyPartitioning.onNoteOffEvent(note);
  if (voice < polyphonyCount) {
    genElements[voice]->onNoteOffEvent(note);
  }
}

void ZSynthController::onSustainOnEvent() {
  for(const auto &elem: genElements) {
    elem->onSustainOnEvent();
  }
}

void ZSynthController::onSustainOffEvent() {
  for(const auto &elem: genElements) {
    elem->onSustainOffEvent();
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
