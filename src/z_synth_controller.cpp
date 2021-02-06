#include "./z_synth_controller.h"

#include "./elements/adsr_element.h"
#include "./elements/amp_element.h"
#include "./elements/distortion_element.h"
#include "./elements/generator_element.h"
#include "./elements/lowpass_filter_element.h"
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

    // adsrs
    char adsrAmpName[20];
    snprintf(adsrAmpName, sizeof(adsrAmpName), "adsr-amp-%d", i);
    builder.registerElem(adsrAmpName, adsrAmpElements[i]);

    char adsrFilterName[20];
    snprintf(adsrFilterName, sizeof(adsrFilterName), "adsr-filt-%d", i);
    builder.registerElem(adsrFilterName, adsrFilterElements[i]);

    // filter
    char filterName[20];
    snprintf(filterName, sizeof(filterName), "filter%d", i);
    builder.registerElem(filterName, filterElements[i]);

    // distortion effect
    char distName[20];
    snprintf(distName, sizeof(distName), "dist%d", i);
    builder.registerElem(distName, distElements[i]);

    // generator mods
    builder.connectElems("lfo", genName, genElements[i]->fmPortNumber());
    builder.connectElems(adsrAmpName, genName, genElements[i]->amPortNumber());

    // filter mods
    builder.connectElems(adsrFilterName, filterName, filterElements[i]->modPortNumber());

    // gen -> filter -> dist -> amp
    builder.connectElems(genName, filterName, filterElements[i]->inputPortNumber());
    builder.connectElems(filterName, distName, distElements[i]->inputPortNumber());
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
    lfoElement(make_shared<GeneratorElement>(params.sampleRateHz, sine_function))
{
  for (uint32_t i = 0; i < polyphonyCount; ++i) {
    auto genElem = make_shared<GeneratorElement>(params.sampleRateHz, square_function);
    genElem->setAmplitude(0.1);
    // genElem->setFMLinearRange(10);
    genElements.emplace_back(genElem);

    auto adsrFilterElem = make_shared<ADSRElement>(params.sampleRateHz);
    adsrFilterElem->setAttackTime(0);
    adsrFilterElem->setDecayTime(.1);
    adsrFilterElem->setSustainLevel(.5);
    adsrFilterElem->setReleaseTime(0.3);
    adsrFilterElements.emplace_back(adsrFilterElem);

    auto adsrAmpElem = make_shared<ADSRElement>(params.sampleRateHz);
    adsrAmpElem->setAttackTime(0.1);
    adsrAmpElem->setDecayTime(0.1);
    adsrAmpElem->setSustainLevel(0.5);
    adsrAmpElem->setReleaseTime(0.3);
    adsrAmpElements.emplace_back(adsrAmpElem);

    distElements.emplace_back(make_shared<DistortionElement>(2, 1, 5));

    filterElements.emplace_back(make_shared<LowpassFilterElement>(params.sampleRateHz, 5000, 26));
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
  genElements[voice]->setFMLinearRange((midiNoteToFreq(note + 1) - midiNoteToFreq(note)) * 0.2);
  genElements[voice]->setEnabled(true);
  adsrAmpElements[voice]->trigger();
  adsrFilterElements[voice]->trigger();
}

void ZSynthController::onNoteOffEvent(unsigned char note) {
  uint32_t voice = polyphonyPartitioning.onNoteOffEvent(note);
  if (voice < polyphonyCount) {
    adsrAmpElements[voice]->release();
    adsrFilterElements[voice]->release();
  }
}

void ZSynthController::onSustainOnEvent() {
  polyphonyPartitioning.onSustainOnEvent();
}

void ZSynthController::onSustainOffEvent() {
  for(const auto &voice: polyphonyPartitioning.onSustainOffEvent()) {
    if (voice < polyphonyCount) {
      adsrAmpElements[voice]->release();
      adsrFilterElements[voice]->release();
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
      std::cout << "Set generator to square" << std::endl;
      value = square_function;
    } else {
      std::cout << "Set generator to sine" << std::endl;
      value = sine_function;
    }
    for (auto &elem: genElements) {
      elem->setValue(value);
    }
  }
  else if (paramHigh == 0x13 && paramLow == 0x38) {
    float cutoffFreq = paramValue / 127. * 10000.;
    std::cout << "Set cutoff frequency to " << cutoffFreq << std::endl;
    for (auto &elem: filterElements) {
      elem->setCutoffFreq(cutoffFreq);
    }
  }
}
