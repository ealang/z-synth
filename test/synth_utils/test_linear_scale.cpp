#include <gtest/gtest.h>
#include "../../src/synth_utils/linear_scale.h"

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
