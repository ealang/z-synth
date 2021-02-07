#include "./z_synth_controller.h"

#include "./elements/adsr_element.h"
#include "./elements/amp_element.h"
#include "./elements/distortion_element.h"
#include "./elements/generator_element.h"
#include "./elements/lowpass_filter_element.h"
#include "./elements/mixer_element.h"
#include "./synth_utils/generator_functions.h"
#include "./synth_utils/midi_note_to_freq.h"
#include "./pipeline/pipeline_builder.h"

using namespace std;

shared_ptr<AudioElement<float>> ZSynthController::makeWiring() const {
  PipelineBuilder<float> builder;

  for (uint32_t i = 0; i < polyphonyCount; ++i) {
    // generator1
    char gen1Name[20];
    snprintf(gen1Name, sizeof(gen1Name), "gen1-%d", i);
    builder.registerElem(gen1Name, genElements1[i]);

    // generator2
    char gen2Name[20];
    snprintf(gen2Name, sizeof(gen2Name), "gen2-%d", i);
    builder.registerElem(gen2Name, genElements2[i]);

    // mixer
    char mixerName[20];
    snprintf(mixerName, sizeof(mixerName), "mixer-%d", i);
    builder.registerElem(mixerName, mixerElements[i]);

    // adsrs
    char adsrAmpName[20];
    snprintf(adsrAmpName, sizeof(adsrAmpName), "adsr-amp-%d", i);
    builder.registerElem(adsrAmpName, adsrAmpElements[i]);

    char adsrFilterName[20];
    snprintf(adsrFilterName, sizeof(adsrFilterName), "adsr-filt-%d", i);
    builder.registerElem(adsrFilterName, adsrFilterElements[i]);

    // filter
    char filterName[20];
    snprintf(filterName, sizeof(filterName), "filter-%d", i);
    builder.registerElem(filterName, filterElements[i]);

    // distortion effect
    char distName[20];
    snprintf(distName, sizeof(distName), "dist-%d", i);
    builder.registerElem(distName, distElements[i]);

    // generator mods
    builder.connectElems("lfo", gen1Name, genElements1[i]->fmPortNumber());
    builder.connectElems("lfo", gen2Name, genElements2[i]->fmPortNumber());

    // filter mods
    builder.connectElems(adsrFilterName, filterName, filterElements[i]->modPortNumber());

    // gen1, gen1 -> mixer -> adsr amp -> filter -> dist -> amp
    builder.connectElems(gen1Name, mixerName, mixerElements[i]->inputPortNumber(0));
    builder.connectElems(gen2Name, mixerName, mixerElements[i]->inputPortNumber(1));
    builder.connectElems(mixerName, adsrAmpName, adsrAmpElements[i]->inputPortNumber());
    builder.connectElems(adsrAmpName, filterName, filterElements[i]->inputPortNumber());

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
    lfoElement(make_shared<GeneratorElement>(params.sampleRateHz, noise_function))
{
  for (uint32_t i = 0; i < polyphonyCount; ++i) {
    auto genElem1 = make_shared<GeneratorElement>(params.sampleRateHz, saw_function);
    genElem1->setAmplitude(0.1);
    genElem1->setEnabled(true);
    genElements1.emplace_back(genElem1);

    auto genElem2 = make_shared<GeneratorElement>(params.sampleRateHz, sine_function);
    genElem2->setAmplitude(0.1);
    genElem2->setEnabled(true);
    genElements2.emplace_back(genElem2);

    auto mixerElem = make_shared<MixerElement>(2);
    mixerElements.emplace_back(mixerElem);

    auto adsrFilterElem = make_shared<ADSRElement>(params.sampleRateHz);
    adsrFilterElem->setAttackTime(0);
    adsrFilterElem->setDecayTime(.2);
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

    filterElements.emplace_back(make_shared<LowpassFilterElement>(params.sampleRateHz, 3500, 26));
  }

  lfoElement->setFrequency(8);
  lfoElement->setEnabled(true);

  _pipeline = makeWiring();
}

std::shared_ptr<AudioElement<float>> ZSynthController::pipeline() const {
  return _pipeline;
}

void ZSynthController::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  MidiNoteListener::injectMidi(midi);
  MidiNRPNListener::injectMidi(midi);
}

void ZSynthController::onNoteOnEvent(unsigned char note, unsigned char) {
  uint32_t voice = polyphonyPartitioning.onNoteOnEvent(note);
  float frequency = midiNoteToFreq(note);
  float frequencyModRange = (midiNoteToFreq(note + 1) - frequency) * 0.2;
  genElements1[voice]->setFrequency(frequency);
  genElements2[voice]->setFrequency(frequency);
  genElements1[voice]->setFMLinearRange(frequencyModRange);
  genElements2[voice]->setFMLinearRange(frequencyModRange);

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
    for (auto &elem: genElements1) {
      elem->setValue(value);
    }
  } else if (paramHigh == 0x13 && paramLow == 0x38) {
    float cutoffFreq = paramValue / 127. * 10000.;
    std::cout << "Set cutoff frequency to " << cutoffFreq << std::endl;
    for (auto &elem: filterElements) {
      elem->setCutoffFreq(cutoffFreq);
    }
  } else if (paramHigh == 0x13 && paramLow == 0x39) {
    float mix = paramValue / 127.;
    std::cout << "Set mix to " << mix << std::endl;
    for (auto &mixer: mixerElements) {
      mixer->setWeight(0, mix);
      mixer->setWeight(1, 1 - mix);
    }
  }
}
