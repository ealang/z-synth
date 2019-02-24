#ifndef PIPELINE_SETUP_H
#define PIPELINE_SETUP_H

#include <memory>
#include "./audio_params.h"
#include "./pipeline/pipeline_element.h"

std::shared_ptr<MidiAudioElement<float>> build_pipeline(AudioParams params);

#endif
