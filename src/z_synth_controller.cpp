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

  for (uint32_t i = 0; i < polyphonyCount; ++i) {
    // generator
    char genName[20];
    snprintf(genName, sizeof(genName), "gen%d", i);
    builder.registerElem(genName, genElements[i]);

    // amp envelope
    char adsrName[20];
    snprintf(adsrName, sizeof(adsrName), "adsr%d", i);
    builder.registerElem(adsrName, adsrElements[i]);

    // distortion effect
    char distName[20];
    snprintf(distName, sizeof(distName), "dist%d", i);
    builder.registerElem(distName, distElements[i]);

    builder.connectElems("lfo", genName, genElements[i]->fmPortNumber());

    builder.connectElems(adsrName, genName, genElements[i]->amPortNumber());
    builder.connectElems(adsrName, distName, distElements[i]->modPortNumber());

    builder.connectElems(genName, distName, distElements[i]->inputPortNumber());
    builder.connectElems(distName, "amp", ampElement->inputPortNumber(i));
  }

  builder.registerElem("lfo", lfoElement);
  builder.registerElem("amp", ampElement);
  builder.setOutputElem("amp");

  return builder.build(params.bufferSampleCount);
}

ZSynthController::ZSynthController(AudioParams params)
  : params(params),
    polyphonyPartitioning(polyphonyCount),
    ampElement(make_shared<AmpElement>(0.1)),
    lfoElement(make_shared<GeneratorElement>(params.sampleRateHz, square_function))
{
  for (uint32_t i = 0; i < polyphonyCount; ++i) {
    auto genElem = make_shared<GeneratorElement>(params.sampleRateHz, sine_function);
    genElem->setAmplitude(0.1);
    genElements.emplace_back(genElem);

    auto adsrElem = make_shared<ADSRElement>(params.sampleRateHz);
    adsrElem->setAttackTime(0.1);
    adsrElem->setDecayTime(0.1);
    adsrElem->setSustainLevel(0.5);
    adsrElem->setReleaseTime(0.3);
    adsrElements.emplace_back(adsrElem);

    distElements.emplace_back(make_shared<DistortionElement>(2, 1, 5));
  }

  lfoElement->setFrequency(8);
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
  genElements[voice]->setFMLinearRange((midiNoteToFreq(note + 1) - midiNoteToFreq(note)) * 0.25);
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
    function<float(float)> value;
    if (paramValue == 0) {
      value = square_function;
    } else {
      value = sine_function;
    }
    for (auto &elem: genElements) {
      elem->setValue(value);
    }
  }
}
