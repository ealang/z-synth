#include "./adsr_element.h"

#include <cstring>
#include <cmath>

static float computeSlope(float sign, float delta, float time, uint32_t sampleRateHz) {
  if (time == 0) {
    return 0;
  }
  float slope = delta / (time * sampleRateHz);
  return slope * (sign >= 0 ? 1 : -1);
}

static uint32_t numStepsToValue(float startValue, float targetValue, float valueStep) {
  float delta = targetValue - startValue;
  if (delta == 0 || valueStep == 0) {
    return 0;
  }
  return round(delta / valueStep);
}

ADSRElement::ADSRElement(uint32_t sampleRateHz)
  : sampleRateHz(sampleRateHz) {}

uint32_t ADSRElement::maxInputs() const {
  return 1;
}

uint32_t ADSRElement::inputPortNumber() const {
  return 0;
}

void ADSRElement::setMaxAmplitude(float maxAmplitude) {
  _maxAmplitude = maxAmplitude;
}

void ADSRElement::setAttackTime(float attackTime) {
  _attackTime = attackTime;
}

void ADSRElement::setDecayTime(float decayTime) {
  _decayTime = decayTime;
}

void ADSRElement::setSustainLevel(float sustainLevel) {
  _sustainLevel = sustainLevel;
}

void ADSRElement::setReleaseTime(float releaseTime) {
  _releaseTime = releaseTime;
}

void ADSRElement::nextState() {
  if (state == State::reset) {
    state = State::attack;
    const float target = _decayTime > 0 ? 1 : _sustainLevel;
    if (_attackTime > 0) {
      valueStep = computeSlope(target - curValue, target, _attackTime, sampleRateHz);
      countdown = numStepsToValue(curValue, target, valueStep);
      return;
    }
    curValue = target;
  }

  if (state == State::attack) {
    state = State::decay;
    if (_decayTime > 0) {
      const float target = _sustainLevel;
      valueStep = computeSlope(target - curValue, 1 - _sustainLevel, _decayTime, sampleRateHz);
      countdown = numStepsToValue(curValue, target, valueStep);
      return;
    }

    curValue = _sustainLevel;
    valueStep = 0;
    countdown = 0;
    return;
  }

  if (state == State::release) {
    if (_releaseTime > 0) {
      const float target = 0;
      valueStep = computeSlope(target - curValue, _sustainLevel, _releaseTime, sampleRateHz);
      countdown = numStepsToValue(curValue, target, valueStep);
      return;
    }
    curValue = 0;
    valueStep = 0;
    countdown = 0;
  }
}

void ADSRElement::trigger() {
  state = State::reset;
  nextState();
}

void ADSRElement::release() {
  state = State::release;
  nextState();
}

void ADSRElement::generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) {
  float const* in = numInputs > 0 ? inputs[0] : nullptr;
  for (uint32_t i = 0; i < numSamples; ++i) {
    *(out++) = curValue * _maxAmplitude * (in != nullptr ? (*in++) : 1);

    if (countdown > 0) {
      --countdown;
      curValue += valueStep;
      if (state != State::release && countdown == 0) {
        nextState();
      }
    }
  }
};

