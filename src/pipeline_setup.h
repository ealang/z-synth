#ifndef PIPELINE_SETUP_H
#define PIPELINE_SETUP_H

#include <cstdint>
#include <memory>
#include "./pipeline/pipeline_element.h"

std::shared_ptr<MidiAudioElement<float>> build_pipeline(uint32_t sampleRateHz, uint32_t bufferSize, uint32_t channelCount);

#endif
