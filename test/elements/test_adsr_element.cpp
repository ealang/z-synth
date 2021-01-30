#include "../../src/elements/adsr_element.h"
#include "./expect_buffer_eq.h"

#include <gtest/gtest.h>
#include <vector>

TEST(ADSRElementTest, givenInitialState_ItOutputs0) {
  ADSRElement element(1);
  expect_buffer_eq({0, 0, 0}, element);
}

TEST(ADSRElementTest, givenNormalOperation_ItExecutesADSR) {
  ADSRElement element(1);
  element.setAttackTime(5);
  element.setDecayTime(4);
  element.setSustainLevel(.6);
  element.setMaxAmplitude(10);
  element.setReleaseTime(3);

  element.trigger();
  expect_buffer_eq({0, 2, 4, 6, 8, 10, 9, 8, 7, 6, 6, 6, 6}, element);

  element.release();
  expect_buffer_eq({6, 4, 2, 0, 0, 0}, element);
}

TEST(ADSRElementTest, givenNoAttack_ItOutputsDecay) {
  ADSRElement element(1);
  element.setAttackTime(0);
  element.setDecayTime(5);
  element.setSustainLevel(.5);
  element.setMaxAmplitude(10);

  element.trigger();
  expect_buffer_eq({10, 9, 8, 7, 6, 5, 5, 5}, element);
}

TEST(ADSRElementTest, givenNoAttackOrSustain_ItEntersSustain) {
  ADSRElement element(1);
  element.setAttackTime(0);
  element.setDecayTime(0);
  element.setSustainLevel(.5);
  element.setMaxAmplitude(10);

  element.trigger();
  expect_buffer_eq({5, 5, 5, 5, 5}, element);
}

TEST(ADSRElementTest, givenNoDecay_ItApproachesSustain) {
  ADSRElement element(1);
  element.setAttackTime(5);
  element.setDecayTime(0);
  element.setSustainLevel(.5);
  element.setMaxAmplitude(10);

  element.trigger();
  expect_buffer_eq({0, 1, 2, 3, 4, 5, 5, 5}, element);
}

TEST(ADSRElementTest, givenNoRelease_ItDropsToZero) {
  ADSRElement element(1);
  element.setAttackTime(0);
  element.setDecayTime(0);
  element.setReleaseTime(0);
  element.setSustainLevel(1);
  element.setMaxAmplitude(1);

  element.trigger();
  expect_buffer_eq({1, 1, 1, 1}, element);

  element.release();
  expect_buffer_eq({0, 0, 0, 0}, element);
}

TEST(ADSRElementTest, givenFastOnOff_ItStaysZero) {
  ADSRElement element(1);
  element.setAttackTime(1);
  element.setDecayTime(1);
  element.setSustainLevel(1);
  element.setMaxAmplitude(1);

  element.trigger();
  element.release();
  expect_buffer_eq({0, 0, 0, 0}, element);
}

TEST(ADSRElementTest, givenRepeatAttack_ItResets) {
  ADSRElement element(1);
  element.setAttackTime(5);
  element.setDecayTime(5);
  element.setSustainLevel(.5);
  element.setMaxAmplitude(10);

  element.trigger();
  expect_buffer_eq({0, 2, 4, 6}, element);
  element.release();
  element.trigger();
  expect_buffer_eq({8, 10, 9, 8, 7, 6, 5, 5}, element);
}

TEST(ADSRElementTest, givenRepeatAttackAndParameterChange_ItResets) {
  ADSRElement element(1);
  element.setAttackTime(5);
  element.setDecayTime(5);
  element.setSustainLevel(.5);
  element.setMaxAmplitude(10);

  element.trigger();
  expect_buffer_eq({0, 2, 4, 6}, element);

  element.setAttackTime(10);

  element.release();
  element.trigger();
  expect_buffer_eq({8, 9, 10, 9, 8, 7, 6, 5, 5}, element);
}

TEST(ADSRElementTest, givenReleaseDuringAttack_ItGoesToZero) {
  ADSRElement element(1);
  element.setAttackTime(10);
  element.setDecayTime(0);
  element.setSustainLevel(1);
  element.setMaxAmplitude(10);
  element.setReleaseTime(10);

  element.trigger();
  expect_buffer_eq({0, 1, 2, 3, 4, 5}, element);

  element.release();
  expect_buffer_eq({6, 5, 4, 3, 2, 1, 0, 0}, element);
}
