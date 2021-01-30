#ifndef EXPECT_BUFFER_EQ_H
#define EXPECT_BUFFER_EQ_H

#include "../../src/pipeline/pipeline_element.h"

#include <vector>

void expect_buffer_eq(std::vector<float> expected, AudioElement<float>& elem, float tolerance = 0.001);

#endif
