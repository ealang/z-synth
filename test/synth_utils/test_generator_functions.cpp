#include "../../src/synth_utils/generator_functions.h"
#include <gtest/gtest.h>

void EXPECT_CLOSE(float exp, float actual) {
  EXPECT_TRUE(abs(exp - actual) < 0.001) << exp << " vs " << actual;
}

TEST(TriangleFunction, ItGeneratesATriangleWave) {
  EXPECT_CLOSE(0, triangle_function(0));
  EXPECT_CLOSE(.4, triangle_function(.1));
  EXPECT_CLOSE(1, triangle_function(.25));
  EXPECT_CLOSE(.6, triangle_function(.35));

  EXPECT_CLOSE(0, triangle_function(.5));
  EXPECT_CLOSE(-.4, triangle_function(.6));
  EXPECT_CLOSE(-1, triangle_function(.75));
  EXPECT_CLOSE(-.6, triangle_function(.85));

  EXPECT_CLOSE(0, triangle_function(1));
}

TEST(SawFunction, ItGeneratesASawWave) {
  EXPECT_CLOSE(0, saw_function(0));
  EXPECT_CLOSE(.2, saw_function(.1));
  EXPECT_CLOSE(-1, saw_function(.5));
  EXPECT_CLOSE(-.8, saw_function(.6));
  EXPECT_CLOSE(0, saw_function(1));
}
