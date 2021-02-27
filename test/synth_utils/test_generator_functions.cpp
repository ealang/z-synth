#include "../../src/synth_utils/generator_functions.h"
#include <gtest/gtest.h>

void EXPECT_CLOSE(float exp, float actual) {
  EXPECT_TRUE(abs(exp - actual) < 0.001) << exp << " vs " << actual;
}

TEST(TriangleFunction, ItGeneratesATriangleWave) {
  EXPECT_CLOSE(-1, triangle_function(0));
  EXPECT_CLOSE(0, triangle_function(.25));
  EXPECT_CLOSE(1, triangle_function(.5));
  EXPECT_CLOSE(0, triangle_function(.75));
  EXPECT_CLOSE(-1, triangle_function(1));
}

TEST(SawFunction, ItGeneratesASawWave) {
  EXPECT_CLOSE(-1, saw_function(0));
  EXPECT_CLOSE(-0.5, saw_function(.25));
  EXPECT_CLOSE(0, saw_function(.5));
  EXPECT_CLOSE(0.5, saw_function(.75));
  EXPECT_CLOSE(1, saw_function(1));
}

TEST(ReverseSawFunction, ItGeneratesAReverseSawWave) {
  EXPECT_CLOSE(1, reverse_saw_function(0));
  EXPECT_CLOSE(0.5, reverse_saw_function(.25));
  EXPECT_CLOSE(0, reverse_saw_function(.5));
  EXPECT_CLOSE(-0.5, reverse_saw_function(.75));
  EXPECT_CLOSE(-1, reverse_saw_function(1));
}
