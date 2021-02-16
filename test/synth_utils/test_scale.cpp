#include "../../src/synth_utils/scale.h"
#include <gtest/gtest.h>

TEST(LinearScale, givenValueOutOfDomain_ItClampToClosestValue) {
  float left = 10;
  float right = 100;
  float start = 1;
  float end = 2;

  auto scale = linearScaleClamped(left, right, start, end);
  ASSERT_EQ(scale(left - 1), start);
  ASSERT_EQ(scale(right + 1), end);
}

TEST(LinearScale, givenValueWithinDomain_ItLinearlyInterpolates) {
  float left = 10;
  float right = 20;
  float start = 0;
  float end = 100;

  auto scale = linearScaleClamped(left, right, start, end);
  ASSERT_EQ(scale(left), start);
  ASSERT_EQ(scale(15), 50);
  ASSERT_EQ(scale(right), end);
}

TEST(PowerScaleLeftRight, givenValueOutOfDomain_ItClampToClosestValue) {
  float left = 10;
  float right = 100;
  float start = 10;
  float end = 20;

  auto scale = powerScaleClamped(1, left, right, start, end);
  ASSERT_EQ(scale(left - 1), start);
  ASSERT_EQ(scale(right + 1), end);
}

TEST(PowerScaleRightLeft, givenValueOutOfDomain_ItClampToClosestValue) {
  float left = 100;
  float right = 10;
  float start = 1;
  float end = 2;

  auto scale = powerScaleClamped(1, left, right, start, end);
  ASSERT_EQ(scale(left + 1), start);
  ASSERT_EQ(scale(right - 1), end);
}

TEST(PowerScale, givenValueWithinDomain_ItInterpolates) {
  float left = 10;
  float right = 20;
  float start = 100;
  float end = 200;

  auto scale = powerScaleClamped(1, left, right, start, end);
  ASSERT_EQ(scale(left), start);
  ASSERT_TRUE(abs(scale(15) - 133.33) < 0.1);
  ASSERT_EQ(scale(right), end);
}
