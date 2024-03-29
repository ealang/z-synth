#include "./z_synth_controller.h"

#include "./elements/adsr_element.h"
#include "./elements/amp_element.h"
#include "./elements/generator_element.h"
#include "./elements/lowpass_filter_element.h"
#include "./elements/math_mult_add_element.h"
#include "./elements/mixer_element.h"
#include "./elements/threaded_mixer_element.h"
#include "./synth_utils/generator_functions.h"
#include "./synth_utils/midi_note_to_freq.h"
#include "./synth_utils/scale.h"
#include "./pipeline/pipeline_builder.h"

#include <cmath>
#include <optional>

// Defaults and ranges

static constexpr float filterMinCutoffHz = 50;
static constexpr float filterMaxCutoffHz = 20000;
static constexpr float maxFmSemiToneRange = 3;
static constexpr float envelopeMinAttack = 0.01;
static constexpr float envelopeMaxAttack = 1;
static constexpr float envelopeMaxDecay = 1;
static constexpr float envelopeMinRelease = 0.01;
static constexpr float envelopeMaxRelease = 5;

static constexpr float minLfoFrequencyHz = 0.1;
static constexpr float maxLfoFrequencyHz = 50;
static constexpr float defaultLfoFreqHz = 8;
static constexpr float defaultLfoSendGenFM = 0.1;
static constexpr float defaultLfoSendGenAm = 0;
static constexpr float defaultLfoSendFilterCutoff = 0;

static constexpr float maxMasterAmp = 1;
static constexpr float defaultMasterAmp = maxMasterAmp * 0.5;
static constexpr float defaultMasterOverdrive = 1;
static constexpr float overdriveMultiplier = 64;
static constexpr float maxGeneratorMixerWeight = 3;

static constexpr float fmSemitoneRange = 1;
static constexpr float lfoMaxModValue = 1;

// Param numbers

static constexpr uint8_t NRPN_MSB_VALUE = 0x10;

static constexpr uint8_t PARAM_GEN1_WAVE_TYPE     = 0x00;
static constexpr uint8_t PARAM_GEN2_WAVE_TYPE     = 0x01;
static constexpr uint8_t PARAM_GEN1_AMP           = 0x02;
static constexpr uint8_t PARAM_FILTER_CUTOFF      = 0x03;
static constexpr uint8_t PARAM_GEN1_FINE_OFFSET   = 0x04;
static constexpr uint8_t PARAM_GEN2_FINE_OFFSET   = 0x05;
static constexpr uint8_t PARAM_AMP_ENV_ATTACK     = 0x06;
static constexpr uint8_t PARAM_AMP_ENV_DECAY      = 0x07;
static constexpr uint8_t PARAM_AMP_ENV_SUSTAIN    = 0x08;
static constexpr uint8_t PARAM_AMP_ENV_RELEASE    = 0x09;
static constexpr uint8_t PARAM_FILTER_ENV_ATTACK  = 0x0A;
static constexpr uint8_t PARAM_FILTER_ENV_DECAY   = 0x0B;
static constexpr uint8_t PARAM_FILTER_ENV_SUSTAIN = 0x0C;
static constexpr uint8_t PARAM_FILTER_ENV_RELEASE = 0x0D;
static constexpr uint8_t PARAM_DISTORTION         = 0x0E;
static constexpr uint8_t PARAM_LFO_FREQ           = 0x10;
static constexpr uint8_t PARAM_LFO_WAVE_TYPE      = 0x11;
static constexpr uint8_t PARAM_MASTER_AMP         = 0x12;
static constexpr uint8_t PARAM_GEN1_COARSE_OFFSET = 0x13;
static constexpr uint8_t PARAM_GEN2_COARSE_OFFSET = 0x14;
static constexpr uint8_t PARAM_GEN3_FINE_OFFSET   = 0x15;
static constexpr uint8_t PARAM_GEN3_COARSE_OFFSET = 0x16;
static constexpr uint8_t PARAM_GEN3_WAVE_TYPE     = 0x17;
static constexpr uint8_t PARAM_GEN2_AMP           = 0x18;
static constexpr uint8_t PARAM_GEN3_AMP           = 0x19;
static constexpr uint8_t PARAM_LFO_MOD_FREQ_AMT   = 0x1A;
static constexpr uint8_t PARAM_LFO_MOD_FILT_AMT   = 0x1B;
static constexpr uint8_t PARAM_LFO_MOD_AMP_AMT    = 0x1C;
static constexpr uint8_t PARAM_LFO_MOD_PARAM_SEL  = 0x1D;
static constexpr uint8_t PARAM_LFO_MOD_PARAM_AMT  = 0x1E;

static const std::vector<std::function<float(float)>> generatorTable {
  square_function,
  sine_function,
  triangle_function,
  noise_function,
  saw_function,
  sampled_noise(8),
  reverse_saw_function,
  zero_function,
};

static const std::vector<std::string> generatorTableNames {
  "square",
  "sine",
  "triangle",
  "noise",
  "saw",
  "sampled noise 8",
  "reverse saw",
  "(disabled)",
};

class NullBuffer : public std::streambuf
{
public:
  int overflow(int c) { return c; }
};

// Monophonic pipeline
class PerVoiceController {
  // Component elements
  std::shared_ptr<GeneratorElement> lfoElement;
  std::shared_ptr<MathMultAddElement> lfoSendGenFMElement;
  std::shared_ptr<MathMultAddElement> lfoSendGenAmElement;
  std::shared_ptr<MathMultAddElement> lfoSendFilterCutoffElement;
  std::shared_ptr<GeneratorElement> genElement1;
  std::shared_ptr<GeneratorElement> genElement2;
  std::shared_ptr<GeneratorElement> genElement3;
  std::shared_ptr<MixerElement> mixerElement;
  std::shared_ptr<ADSRElement> adsrAmpElement;
  std::shared_ptr<ADSRElement> adsrFilterElement;
  std::shared_ptr<LowpassFilterElement> filterElement;

  std::optional<uint32_t> lfoModParamSelect;
  float lfoModParamValue = 0;

  std::vector<std::string> lfoModParamNames;
  std::vector<MathMultAddElement*> lfoModParamElems;

  unsigned char lastNote = 0;

  // logical element/wiring
  std::shared_ptr<AudioElement<float>> _pipeline;

  void constructElems(AudioParams params) {
    lfoElement = std::make_shared<GeneratorElement>(params.sampleRateHz, sine_function);
    lfoElement->setFrequency(defaultLfoFreqHz);
    lfoElement->setAmplitude(1);

    lfoSendGenFMElement = std::make_shared<MathMultAddElement>(defaultLfoSendGenFM, 0);
    lfoSendGenAmElement = std::make_shared<MathMultAddElement>(defaultLfoSendGenAm, 1);
    lfoSendFilterCutoffElement = std::make_shared<MathMultAddElement>(defaultLfoSendFilterCutoff, 1);

    genElement1 = std::make_shared<GeneratorElement>(params.sampleRateHz, saw_function);
    genElement2 = std::make_shared<GeneratorElement>(params.sampleRateHz, sine_function);
    genElement3 = std::make_shared<GeneratorElement>(params.sampleRateHz, sine_function);

    uint32_t numElements = 3;
    mixerElement = std::make_shared<MixerElement>(numElements, 1);

    adsrAmpElement = std::make_shared<ADSRElement>(params.sampleRateHz);
    adsrAmpElement->setAttackTime(0.01);
    adsrAmpElement->setDecayTime(0.1);
    adsrAmpElement->setSustainLevel(.5);
    adsrAmpElement->setReleaseTime(0.3);

    adsrFilterElement = std::make_shared<ADSRElement>(params.sampleRateHz);
    adsrFilterElement->setAttackTime(0.1);
    adsrFilterElement->setDecayTime(0.1);
    adsrFilterElement->setSustainLevel(0.5);
    adsrFilterElement->setReleaseTime(0.3);

    filterElement = std::make_shared<LowpassFilterElement>(params.sampleRateHz, 620, 101);

    lfoModParamNames = {
      "am",
      "filter cutoff",
      "(disabled)",
      "(disabled)",
      "(disabled)",
      "(disabled)",
      "(disabled)",
      "(disabled)",
    };
    lfoModParamElems = {
      lfoSendGenAmElement.get(),
      lfoSendFilterCutoffElement.get(),
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
    };

    updateLfoModParamElement();
  }

  void constructWiring(AudioParams params, uint32_t i) {
    PipelineBuilder<float> builder;

    // lfo
    char lfoName[20];
    snprintf(lfoName, sizeof(lfoName), "lfo-%d", i);
    builder.registerElem(lfoName, lfoElement);

    // lfo mod levels
    char lfoSendGenFMName[20];
    snprintf(lfoSendGenFMName, sizeof(lfoSendGenFMName), "lfo-send-fm-%d", i);
    builder.registerElem(lfoSendGenFMName, lfoSendGenFMElement);

    char lfoSendGenAmName[20];
    snprintf(lfoSendGenAmName, sizeof(lfoSendGenAmName), "lfo-send-am-%d", i);
    builder.registerElem(lfoSendGenAmName, lfoSendGenAmElement);

    char lfoSendFilterCutoffName[20];
    snprintf(lfoSendFilterCutoffName, sizeof(lfoSendFilterCutoffName), "lfo-send-filt-%d", i);
    builder.registerElem(lfoSendFilterCutoffName, lfoSendFilterCutoffElement);

    // generator1
    char gen1Name[20];
    snprintf(gen1Name, sizeof(gen1Name), "gen1-%d", i);
    builder.registerElem(gen1Name, genElement1);

    // generator2
    char gen2Name[20];
    snprintf(gen2Name, sizeof(gen2Name), "gen2-%d", i);
    builder.registerElem(gen2Name, genElement2);

    // generator3
    char gen3Name[20];
    snprintf(gen3Name, sizeof(gen3Name), "gen3-%d", i);
    builder.registerElem(gen3Name, genElement3);

    // mixer
    char mixerName[20];
    snprintf(mixerName, sizeof(mixerName), "mixer-%d", i);
    builder.registerElem(mixerName, mixerElement);

    // adsrs
    char adsrAmpName[20];
    snprintf(adsrAmpName, sizeof(adsrAmpName), "adsr-amp-%d", i);
    builder.registerElem(adsrAmpName, adsrAmpElement);

    char adsrFilterName[20];
    snprintf(adsrFilterName, sizeof(adsrFilterName), "adsr-filt-%d", i);
    builder.registerElem(adsrFilterName, adsrFilterElement);

    // filter
    char filterName[20];
    snprintf(filterName, sizeof(filterName), "filter-%d", i);
    builder.registerElem(filterName, filterElement);

    // lfo to sends
    builder.connectElems(lfoName, lfoSendGenFMName, lfoSendGenFMElement->inputPortNumber());
    builder.connectElems(lfoName, lfoSendGenAmName, lfoSendGenAmElement->inputPortNumber());
    builder.connectElems(lfoName, lfoSendFilterCutoffName, lfoSendFilterCutoffElement->inputPortNumber());

    // generator mods
    builder.connectElems(lfoSendGenFMName, gen1Name, genElement1->fmPortNumber());
    builder.connectElems(lfoSendGenFMName, gen2Name, genElement2->fmPortNumber());
    builder.connectElems(lfoSendGenFMName, gen3Name, genElement3->fmPortNumber());
    builder.connectElems(lfoSendGenAmName, gen1Name, genElement1->amPortNumber());
    builder.connectElems(lfoSendGenAmName, gen2Name, genElement2->amPortNumber());
    builder.connectElems(lfoSendGenAmName, gen3Name, genElement3->amPortNumber());

    // filter mods
    builder.connectElems(lfoSendFilterCutoffName, adsrFilterName, adsrFilterElement->inputPortNumber());
    builder.connectElems(adsrFilterName, filterName, filterElement->modPortNumber());

    // gens -> mixer -> adsr amp -> filter
    builder.connectElems(gen1Name, mixerName, mixerElement->inputPortNumber(0));
    builder.connectElems(gen2Name, mixerName, mixerElement->inputPortNumber(1));
    builder.connectElems(gen3Name, mixerName, mixerElement->inputPortNumber(2));
    builder.connectElems(mixerName, adsrAmpName, adsrAmpElement->inputPortNumber());
    builder.connectElems(adsrAmpName, filterName, filterElement->inputPortNumber());

    builder.setOutputElem(filterName);

    _pipeline = builder.build(params.bufferSampleCount);
  }

  void updateGeneratorFrequencyParams() {
    float gen1Offset = gen1SemitoneOffset + gen1FineOffset;
    float gen2Offset = gen2SemitoneOffset + gen2FineOffset;
    float gen3Offset = gen3SemitoneOffset + gen3FineOffset;

    float frequency1 = midiNoteToFreq(lastNote + gen1Offset);
    float frequency2 = midiNoteToFreq(lastNote + gen2Offset);
    float frequency3 = midiNoteToFreq(lastNote + gen3Offset);
    float frequencyModRange1 = (midiNoteToFreq(lastNote + gen1Offset + 1) - frequency1) * fmSemitoneRange;
    float frequencyModRange2 = (midiNoteToFreq(lastNote + gen2Offset + 1) - frequency2) * fmSemitoneRange;
    float frequencyModRange3 = (midiNoteToFreq(lastNote + gen3Offset + 1) - frequency3) * fmSemitoneRange;
    genElement1->setFrequency(frequency1);
    genElement2->setFrequency(frequency2);
    genElement3->setFrequency(frequency3);
    genElement1->setFMLinearRange(frequencyModRange1);
    genElement2->setFMLinearRange(frequencyModRange2);
    genElement3->setFMLinearRange(frequencyModRange3);
  }

  void updateLfoModParamElement() {
    uint32_t selected = (lfoModParamSelect && *lfoModParamSelect < lfoModParamElems.size()) ?
      *lfoModParamSelect : -1;

    for (uint32_t i = 0; i < lfoModParamElems.size(); ++i) {
      float val = i == selected ? lfoModParamValue : 0;
      auto elem = lfoModParamElems[i];
      if (elem) {
        elem->setMultValue(val);
      }
    }
  }

public:

  int gen1SemitoneOffset = 0;
  int gen2SemitoneOffset = 0;
  int gen3SemitoneOffset = 0;
  float gen1FineOffset = 0;
  float gen2FineOffset = 0;
  float gen3FineOffset = 0;

  PerVoiceController(AudioParams params, uint32_t i) {
    constructElems(params);
    constructWiring(params, i);
  }

  std::shared_ptr<AudioElement<float>> pipeline() const {
    return _pipeline;
  }

  void onNoteOnEvent(unsigned char note, unsigned char) {
    lastNote = note;
    updateGeneratorFrequencyParams();
    adsrAmpElement->trigger();
    adsrFilterElement->trigger();
  }

  void onNoteOffEvent() {
    adsrAmpElement->release();
    adsrFilterElement->release();
  }

  void onNRPNValueHighChange(
    unsigned char paramNumber,
    unsigned char paramValue,
    bool print
  ) {

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

    NullBuffer nullBuffer;
    std::ostream nullStream(&nullBuffer);
    std::ostream &logger = print ? std::cout : nullStream;

    if (paramNumber == PARAM_GEN1_WAVE_TYPE) {
      // Generator 1 wave type
      uint32_t waveNum = static_cast<uint32_t>(paramValue);
      if (waveNum < generatorTable.size()) {
        logger << "Set gen 1 wave " << generatorTableNames[waveNum] << std::endl;
        genElement1->setValue(generatorTable[waveNum]);
      }
    } else if (paramNumber == PARAM_GEN2_WAVE_TYPE) {
      // Generator 2 wave type
      uint32_t waveNum = static_cast<uint32_t>(paramValue);
      if (waveNum < generatorTable.size()) {
        logger << "Set gen 2 wave " << generatorTableNames[waveNum] << std::endl;
        genElement2->setValue(generatorTable[waveNum]);
      }
    } else if (paramNumber == PARAM_GEN3_WAVE_TYPE) {
      // Generator 2 wave type
      uint32_t waveNum = static_cast<uint32_t>(paramValue);
      if (waveNum < generatorTable.size()) {
        logger << "Set gen 3 wave " << generatorTableNames[waveNum] << std::endl;
        genElement3->setValue(generatorTable[waveNum]);
      }
    } else if (paramNumber == PARAM_GEN1_AMP) {
      const float mix = normMidi(paramValue);
      logger << "Set gen 1 amp to " << mix << std::endl;
      mixerElement->setWeight(0, mix * maxGeneratorMixerWeight);
    } else if (paramNumber == PARAM_GEN2_AMP) {
      const float mix = normMidi(paramValue);
      logger << "Set gen 2 amp to " << mix << std::endl;
      mixerElement->setWeight(1, mix * maxGeneratorMixerWeight);
    } else if (paramNumber == PARAM_GEN3_AMP) {
      const float mix = normMidi(paramValue);
      logger << "Set gen 3 amp to " << mix << std::endl;
      mixerElement->setWeight(2, mix * maxGeneratorMixerWeight);
    } else if (paramNumber == PARAM_FILTER_CUTOFF) {
      auto scale = powerScaleClamped(3, 0, 127, filterMinCutoffHz, filterMaxCutoffHz);
      float cutoffFreq = scale(paramValue);
      logger << "Set cutoff frequency to " << cutoffFreq << std::endl;
      filterElement->setCutoffFreq(cutoffFreq);
    } else if (paramNumber == PARAM_GEN1_FINE_OFFSET) {
      gen1FineOffset = normMidiEven(paramValue, -1, 1);
      updateGeneratorFrequencyParams();
      logger << "Set gen 1 fine offset to " << gen1FineOffset << std::endl;
    } else if (paramNumber == PARAM_GEN1_COARSE_OFFSET) {
      gen1SemitoneOffset = static_cast<int>(clamp(paramValue, 0, 48)) - 24;
      updateGeneratorFrequencyParams();
      logger << "Set gen 1 semitone offset to " << gen1SemitoneOffset << std::endl;
    } else if (paramNumber == PARAM_GEN2_FINE_OFFSET) {
      gen2FineOffset = normMidiEven(paramValue, -1, 1);
      updateGeneratorFrequencyParams();
      logger << "Set gen 2 fine offset to " << gen2FineOffset << std::endl;
    } else if (paramNumber == PARAM_GEN2_COARSE_OFFSET) {
      gen2SemitoneOffset = static_cast<int>(clamp(paramValue, 0, 48)) - 24;
      updateGeneratorFrequencyParams();
      logger << "Set gen 2 semitone offset to " << gen2SemitoneOffset << std::endl;
    } else if (paramNumber == PARAM_GEN3_FINE_OFFSET) {
      gen3FineOffset = normMidiEven(paramValue, -1, 1);
      updateGeneratorFrequencyParams();
      logger << "Set gen 3 fine offset to " << gen3FineOffset << std::endl;
    } else if (paramNumber == PARAM_GEN3_COARSE_OFFSET) {
      gen3SemitoneOffset = static_cast<int>(clamp(paramValue, 0, 48)) - 24;
      updateGeneratorFrequencyParams();
      logger << "Set gen 3 semitone offset to " << gen3SemitoneOffset << std::endl;
    } else if (paramNumber == PARAM_AMP_ENV_ATTACK) {
      float attack = normMidi(paramValue, envelopeMinAttack, envelopeMaxAttack);
      logger << "Set amp attack time to " << attack << std::endl;
      adsrAmpElement->setAttackTime(attack);
    } else if (paramNumber == PARAM_AMP_ENV_DECAY) {
      float decay = normMidi(paramValue) * envelopeMaxDecay;
      logger << "Set amp decay time to " << decay << std::endl;
      adsrAmpElement->setDecayTime(decay);
    } else if (paramNumber == PARAM_AMP_ENV_SUSTAIN) {
      float sustain = normMidi(paramValue);
      logger << "Set amp sustain level to " << sustain << std::endl;
      adsrAmpElement->setSustainLevel(sustain);
    } else if (paramNumber == PARAM_AMP_ENV_RELEASE) {
      float release = normMidi(paramValue, envelopeMinRelease, envelopeMaxRelease);
      logger << "Set amp release time to " << release << std::endl;
      adsrAmpElement->setReleaseTime(release);
    } else if (paramNumber == PARAM_FILTER_ENV_ATTACK) {
      float attack = normMidi(paramValue, envelopeMinAttack, envelopeMaxAttack);
      logger << "Set filter attack time to " << attack << std::endl;
      adsrFilterElement->setAttackTime(attack);
    } else if (paramNumber == PARAM_FILTER_ENV_DECAY) {
      float decay = normMidi(paramValue) * envelopeMaxDecay;
      logger << "Set filter decay time to " << decay << std::endl;
      adsrFilterElement->setDecayTime(decay);
    } else if (paramNumber == PARAM_FILTER_ENV_SUSTAIN) {
      float sustain = normMidi(paramValue);
      logger << "Set filter sustain level to " << sustain << std::endl;
      adsrFilterElement->setSustainLevel(sustain);
    } else if (paramNumber == PARAM_FILTER_ENV_RELEASE) {
      float release = normMidi(paramValue, envelopeMinRelease, envelopeMaxRelease);
      logger << "Set filter release time to " << release << std::endl;
      adsrFilterElement->setReleaseTime(release);
    } else if (paramNumber == PARAM_LFO_FREQ) {
      float value = normMidi(paramValue, minLfoFrequencyHz, maxLfoFrequencyHz);
      logger << "Set LFO frequency to " << value << std::endl;
      lfoElement->setFrequency(value);
    } else if (paramNumber == PARAM_LFO_WAVE_TYPE) {
      uint32_t waveNum = static_cast<int>(paramValue);
      if (waveNum < generatorTable.size()) {
        logger << "Set LFO wave " << generatorTableNames[waveNum] << std::endl;
        lfoElement->setValue(generatorTable[waveNum]);
      }
    } else if (paramNumber == PARAM_LFO_MOD_FREQ_AMT) {
      float value = normMidi(paramValue, 0, lfoMaxModValue);
      logger << "Set LFO mod gen freq amount " << value << std::endl;
      lfoSendGenFMElement->setMultValue(value);
    } else if (paramNumber == PARAM_LFO_MOD_FILT_AMT) {
      float value = normMidi(paramValue, 0, lfoMaxModValue);
      logger << "Set LFO mod filter cutoff amount " << value << std::endl;
      lfoSendFilterCutoffElement->setMultValue(value);
    } else if (paramNumber == PARAM_LFO_MOD_AMP_AMT) {
      float value = normMidi(paramValue, 0, lfoMaxModValue);
      logger << "Set LFO mod gen amp amount " << value << std::endl;
      lfoSendGenAmElement->setMultValue(value);
    } else if (paramNumber == PARAM_LFO_MOD_PARAM_SEL) {
      uint32_t paramNum = static_cast<uint32_t>(paramValue);
      if (paramNum < lfoModParamNames.size()) {
        lfoModParamSelect = paramNum;
        logger << "Set LFO mod param select to " << lfoModParamNames[paramNum] << std::endl;
        updateLfoModParamElement();
      }
    } else if (paramNumber == PARAM_LFO_MOD_PARAM_AMT) {
      lfoModParamValue = normMidi(paramValue, 0, lfoMaxModValue);
      logger << "Set LFO mod param value to " << lfoModParamValue << std::endl;
      updateLfoModParamElement();
    }
  }
};

static float computeOverdriveOutputScale(float masterOverdrive) {
  // This code is attempting to keep output levels consistent when applying
  // distortion. TODO: Find a more consistent way of handling this.
  constexpr float scale = 0.1;
  return tanh(defaultMasterOverdrive * scale) / tanh(masterOverdrive * scale);
}

ZSynthController::ZSynthController(AudioParams params, uint32_t polyphony, uint32_t numThreads)
  : params(params),
    polyphony(polyphony),
    polyphonyPartitioning(polyphony),
    masterAmp(defaultMasterAmp),
    masterOverdrive(defaultMasterOverdrive)
{

  // Construct elements
  std::vector<std::shared_ptr<AudioElement<float>>> elements;

  for (uint32_t i = 0; i < polyphony; ++i) {
    auto voiceController = std::make_shared<PerVoiceController>(params, i);
    voiceControllers.emplace_back(voiceController);
    elements.emplace_back(voiceController->pipeline());
  }

  mixerElement = std::make_shared<ThreadedMixerElement>(
    params.bufferSampleCount,
    numThreads,
    elements,
    1.0 / elements.size()
  );

  ampElement = std::make_shared<AmpElement>();
  ampElement->setInputAmplificationValue(defaultMasterOverdrive);
  ampElement->setMaxOutputValue(defaultMasterAmp * computeOverdriveOutputScale(defaultMasterOverdrive));

  // Wiring
  PipelineBuilder<float> builder;

  builder.registerElem("voices", mixerElement);
  builder.registerElem("amp", ampElement);
  builder.connectElems("voices", "amp", ampElement->inputPortNumber(0));
  builder.setOutputElem("amp");

  _pipeline = builder.build(params.bufferSampleCount);
}

std::shared_ptr<AudioElement<float>> ZSynthController::pipeline() const {
  return _pipeline;
}

void ZSynthController::injectMidi(Rx::observable<const snd_seq_event_t*> midi) {
  MidiNoteListener::injectMidi(midi);
  MidiNRPNListener::injectMidi(midi);
}

void ZSynthController::onNoteOnEvent(unsigned char note, unsigned char velocity) {
  uint32_t voice = polyphonyPartitioning.onNoteOnEvent(note);
  voiceControllers[voice]->onNoteOnEvent(note, velocity);
}

void ZSynthController::onNoteOffEvent(unsigned char note) {
  uint32_t voice = polyphonyPartitioning.onNoteOffEvent(note);
  if (voice < polyphony) {
    voiceControllers[voice]->onNoteOffEvent();
  }
}

void ZSynthController::onSustainOnEvent() {
  polyphonyPartitioning.onSustainOnEvent();
}

void ZSynthController::onSustainOffEvent() {
  for(const auto &voice: polyphonyPartitioning.onSustainOffEvent()) {
    if (voice < polyphony) {
      voiceControllers[voice]->onNoteOffEvent();
    }
  }
}

void ZSynthController::onNRPNValueHighChange(
  unsigned char paramHigh,
  unsigned char paramLow,
  unsigned char paramValue
) {
  if (paramHigh == NRPN_MSB_VALUE) {
    if (paramLow == PARAM_MASTER_AMP) {
      masterAmp = paramValue / 127. * maxMasterAmp;
      std::cout << "Set master amp " << masterAmp << std::endl;
      ampElement->setMaxOutputValue(masterAmp * computeOverdriveOutputScale(masterOverdrive));
    } else if (paramLow == PARAM_DISTORTION) {
      masterOverdrive = 1 + paramValue / 127. * overdriveMultiplier;
      std::cout << "Set master overdrive param to " << masterOverdrive << std::endl;
      ampElement->setInputAmplificationValue(masterOverdrive);
      ampElement->setMaxOutputValue(masterAmp * computeOverdriveOutputScale(masterOverdrive));
    } else {
      int i = 0;
      for (auto &voice: voiceControllers) {
        voice->onNRPNValueHighChange(paramLow, paramValue, i == 0);
        ++i;
      }
    }
  }
}
