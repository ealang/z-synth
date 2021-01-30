#include "./expect_buffer_eq.h"

#include "../../src/elements/generator_element.h"
#include "../../src/synth_utils/generator_functions.h"

#include <gtest/gtest.h>
#include <vector>

TEST(GeneratorElementTest, givenChangeInFrequencyWithNoFM_OutputIsContinuous) {
  GeneratorElement element(48000, sine_function);
  element.setEnabled(true);

  const float freq1 = 300;
  element.setFrequency(freq1);

  // generate a few samples
  float lastValue;
  {
    std::vector<float> buffer(200);
    element.generate(200, buffer.data(), 0, nullptr);
    lastValue = buffer[199];
  }

  // Change frequency, make sure output is similar
  const float freq2 = 500;
  element.setFrequency(freq2);
  const float tolerance = 0.01;
  expect_buffer_eq({lastValue}, element, tolerance);
}
