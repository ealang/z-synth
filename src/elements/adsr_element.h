#ifndef ADSR_ELEMENT_H
#define ADSR_ELEMENT_H

#include "../pipeline/pipeline_element.h"

/**
 * Emit an attack-decay-sustain-repease signal.
 */
class ADSRElement: public AudioElement<float> {
  enum class State { reset, attack, decay, release };

  uint32_t sampleRateHz;
  float _maxAmplitude = 1;
  float _attackTime = 0.1;
  float _decayTime = 0.1;
  float _releaseTime = 1;
  float _sustainLevel = .8f;

  State state = State::release;
  float valueStep = 0;
  float curValue = 0;
  uint32_t countdown = 0;

  void nextState();

public:
  ADSRElement(uint32_t sampleRateHz);
  uint32_t maxInputs() const override;

  uint32_t inputPortNumber() const;

  void setMaxAmplitude(float maxAmplitude);
  void setAttackTime(float attackTime);
  void setDecayTime(float decayTime);
  void setSustainLevel(float sustainLevel);
  void setReleaseTime(float releaseTime);

  void trigger();
  void release();

  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
