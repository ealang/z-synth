#include "./expect_buffer_eq.h"

#include <gtest/gtest.h>

void expect_buffer_eq(std::vector<float> expected, AudioElement<float>& elem, float tolerance) {
  uint32_t num = expected.size();
  std::vector<float> buffer(num);
  elem.generate(num, buffer.data(), 0, nullptr);
  for (uint32_t i = 0; i < num; ++i) {
    auto expected_sample = expected[i];
    auto actual_sample = buffer[i];
    EXPECT_TRUE(abs(expected_sample - actual_sample) < tolerance) << "Index " << i << ": Expected " << expected_sample << ", got " << actual_sample;
  }
}
