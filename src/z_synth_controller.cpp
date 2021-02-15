#include "./z_synth_controller.h"

#include "./elements/adsr_element.h"
#include "./elements/amp_element.h"
#include "./elements/distortion_element.h"
#include "./elements/generator_element.h"
#include "./elements/lowpass_filter_element.h"
#include "./elements/mixer_element.h"
#include "./synth_utils/generator_functions.h"
#include "./synth_utils/linear_scale.h"
#include "./synth_utils/midi_note_to_freq.h"
#include "./pipeline/pipeline_builder.h"

using namespace std;

static const uint32_t polyphonyCount = 32;

static const float filterMinCutoffHz = 50;
static const float filterMaxCutoffHz = 10000;
static const float maxFmSemiToneRange = 3;
static const float envelopeMinAttack = 0.01;
static const float envelopeMaxAttack = 1;
static const float envelopeMaxDecay = 1;
static const float envelopeMinRelease = 0.01;
static const float envelopeMaxRelease = 5;
static const float maxDistortionParam = 5;
static const float minLFOFrequencyHz = 0.1;
static const float maxLFOFrequencyHz = 50;
static const float maxLFOAmp = 1;

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

    filterElements.emplace_back(make_shared<LowpassFilterElement>(params.sampleRateHz, 3500, 101));
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

  float gen1Offset = gen1SemitoneOffset + gen1FineOffset;
  float gen2Offset = gen2SemitoneOffset + gen2FineOffset;

  float frequency1 = midiNoteToFreq(note + gen1Offset);
  float frequency2 = midiNoteToFreq(note + gen2Offset);
  float frequencyModRange1 = (midiNoteToFreq(note + gen1Offset + 1) - frequency1) * fmSemitoneRange;
  float frequencyModRange2 = (midiNoteToFreq(note + gen2Offset + 1) - frequency2) * fmSemitoneRange;
  genElements1[voice]->setFrequency(frequency1);
  genElements2[voice]->setFrequency(frequency2);
  genElements1[voice]->setFMLinearRange(frequencyModRange1);
  genElements2[voice]->setFMLinearRange(frequencyModRange2);

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
  static vector<function<float(float)>> generatorTable {
    square_function,
    sine_function,
    triangle_function,
    noise_function,
    saw_function
  };

  const uint8_t NRPN_MSB_VALUE = 0x10;

  const uint8_t PARAM_GEN1_WAVE_TYPE     = 0x00;
  const uint8_t PARAM_GEN2_WAVE_TYPE     = 0x01;
  const uint8_t PARAM_GEN_MIX            = 0x02;
  const uint8_t PARAM_FILTER_CUTOFF      = 0x03;
  const uint8_t PARAM_GEN1_FINE_OFFSET   = 0x04;
  const uint8_t PARAM_GEN2_FINE_OFFSET   = 0x05;
  const uint8_t PARAM_AMP_ENV_ATTACK     = 0x06;
  const uint8_t PARAM_AMP_ENV_DECAY      = 0x07;
  const uint8_t PARAM_AMP_ENV_SUSTAIN    = 0x08;
  const uint8_t PARAM_AMP_ENV_RELEASE    = 0x09;
  const uint8_t PARAM_FILTER_ENV_ATTACK  = 0x0A;
  const uint8_t PARAM_FILTER_ENV_DECAY   = 0x0B;
  const uint8_t PARAM_FILTER_ENV_SUSTAIN = 0x0C;
  const uint8_t PARAM_FILTER_ENV_RELEASE = 0x0D;
  const uint8_t PARAM_DISTORTION         = 0x0E;
  const uint8_t PARAM_LFO_AMP            = 0x0F;
  const uint8_t PARAM_LFO_FREQ           = 0x10;
  const uint8_t PARAM_LFO_WAVE_TYPE      = 0x11;
  const uint8_t PARAM_MASTER_AMP         = 0x12;
  const uint8_t PARAM_GEN1_COARSE_OFFSET = 0x13;
  const uint8_t PARAM_GEN2_COARSE_OFFSET = 0x14;

  const auto normMidi = [](float val, float start = 0, float end = 1) {
    return linearScaleClamped(0, 127, start, end)(val);
  };

  const auto normMidiEven = [](float val, float start = 0, float end = 1) {
    return linearScaleClamped(0, 128, start, end)(val);
  };

  const auto clamp = [](uint8_t val, uint8_t min, uint8_t max) {
    if (val < min) {
      return min;
    }
    if (val > max) {
      return max;
    }
    return val;
  };

  if (paramHigh == NRPN_MSB_VALUE) {
    if (paramLow == PARAM_GEN1_WAVE_TYPE) {
      // Generator 1 wave type
      if (paramValue < generatorTable.size()) {
        std::cout << "Set gen 1 wave " << static_cast<int>(paramValue) << std::endl;
        for (auto &elem: genElements1) {
          elem->setValue(generatorTable[paramValue]);
        }
      }
    } else if (paramLow == PARAM_GEN2_WAVE_TYPE) {
      // Generator 2 wave type
      if (paramValue < generatorTable.size()) {
        std::cout << "Set gen 2 wave " << static_cast<int>(paramValue) << std::endl;
        for (auto &elem: genElements2) {
          elem->setValue(generatorTable[paramValue]);
        }
      }
    } else if (paramLow == PARAM_GEN_MIX) {
      // Generator 1-2 mix
      const float mix = normMidi(paramValue);
      std::cout << "Set mix to " << mix << std::endl;
      for (auto &mixer: mixerElements) {
        mixer->setWeight(0, 1 - mix);
        mixer->setWeight(1, mix);
      }
    } else if (paramLow == PARAM_FILTER_CUTOFF) {
      float cutoffFreq = normMidi(paramValue, filterMinCutoffHz, filterMaxCutoffHz);
      std::cout << "Set cutoff frequency to " << cutoffFreq << std::endl;
      for (auto &elem: filterElements) {
        elem->setCutoffFreq(cutoffFreq);
      }
    } else if (paramLow == PARAM_GEN1_FINE_OFFSET) {
      gen1FineOffset = normMidiEven(paramValue, -1, 1);
      std::cout << "Set gen 1 fine offset to " << gen1FineOffset << std::endl;
    } else if (paramLow == PARAM_GEN1_COARSE_OFFSET) {
      gen1SemitoneOffset = static_cast<int>(clamp(paramValue, 0, 48)) - 24;
      std::cout << "Set gen 1 semitone offset to " << gen1SemitoneOffset << std::endl;
    } else if (paramLow == PARAM_GEN2_FINE_OFFSET) {
      gen2FineOffset = normMidiEven(paramValue, -1, 1);
      std::cout << "Set gen 2 fine offset to " << gen2FineOffset << std::endl;
    } else if (paramLow == PARAM_GEN2_COARSE_OFFSET) {
      gen2SemitoneOffset = static_cast<int>(clamp(paramValue, 0, 48)) - 24;
      std::cout << "Set gen 2 semitone offset to " << gen2SemitoneOffset << std::endl;
    } else if (paramLow == PARAM_AMP_ENV_ATTACK) {
      float attack = normMidi(paramValue, envelopeMinAttack, envelopeMaxAttack);
      std::cout << "Set amp attack time to " << attack << std::endl;
      for (auto &elem: adsrAmpElements) {
        elem->setAttackTime(attack);
      }
    } else if (paramLow == PARAM_AMP_ENV_DECAY) {
      float decay = normMidi(paramValue) * envelopeMaxDecay;
      std::cout << "Set amp decay time to " << decay << std::endl;
      for (auto &elem: adsrAmpElements) {
        elem->setDecayTime(decay);
      }
    } else if (paramLow == PARAM_AMP_ENV_SUSTAIN) {
      float sustain = normMidi(paramValue);
      std::cout << "Set amp sustain level to " << sustain << std::endl;
      for (auto &elem: adsrAmpElements) {
        elem->setSustainLevel(sustain);
      }
    } else if (paramLow == PARAM_AMP_ENV_RELEASE) {
      float release = normMidi(paramValue, envelopeMinRelease, envelopeMaxRelease);
      std::cout << "Set amp release time to " << release << std::endl;
      for (auto &elem: adsrAmpElements) {
        elem->setReleaseTime(release);
      }
    } else if (paramLow == PARAM_FILTER_ENV_ATTACK) {
      float attack = normMidi(paramValue, envelopeMinAttack, envelopeMaxAttack);
      std::cout << "Set filter attack time to " << attack << std::endl;
      for (auto &elem: adsrFilterElements) {
        elem->setAttackTime(attack);
      }
    } else if (paramLow == PARAM_FILTER_ENV_DECAY) {
      float decay = normMidi(paramValue) * envelopeMaxDecay;
      std::cout << "Set filter decay time to " << decay << std::endl;
      for (auto &elem: adsrFilterElements) {
        elem->setDecayTime(decay);
      }
    } else if (paramLow == PARAM_FILTER_ENV_SUSTAIN) {
      float sustain = normMidi(paramValue);
      std::cout << "Set filter sustain level to " << sustain << std::endl;
      for (auto &elem: adsrFilterElements) {
        elem->setSustainLevel(sustain);
      }
    } else if (paramLow == PARAM_FILTER_ENV_RELEASE) {
      float release = normMidi(paramValue, envelopeMinRelease, envelopeMaxRelease);
      std::cout << "Set filter release time to " << release << std::endl;
      for (auto &elem: adsrFilterElements) {
        elem->setReleaseTime(release);
      }
    } else if (paramLow == PARAM_DISTORTION) {
      float value = normMidi(paramValue) * maxDistortionParam;
      std::cout << "Set distortion param to " << value << std::endl;
      for (auto &elem: distElements) {
        elem->setDefaultAmount(value);
      }
    } else if (paramLow == PARAM_LFO_AMP) {
      float value = normMidi(paramValue) * maxLFOAmp;
      std::cout << "Set LFO amp to " << value << std::endl;
      lfoElement->setAmplitude(value);
    } else if (paramLow == PARAM_LFO_FREQ) {
      float value = normMidi(paramValue, minLFOFrequencyHz, maxLFOFrequencyHz);
      std::cout << "Set LFO frequency to " << value << std::endl;
      lfoElement->setFrequency(value);
    } else if (paramLow == PARAM_LFO_WAVE_TYPE) {
      if (paramValue < generatorTable.size()) {
        std::cout << "Set LFO wave " << static_cast<int>(paramValue) << std::endl;
        lfoElement->setValue(generatorTable[paramValue]);
      }
    } else if (paramLow == PARAM_MASTER_AMP) {
      float value = normMidi(paramValue);
      std::cout << "Set master amp " << value << std::endl;
      ampElement->setAmp(value);
    }
  }
}
