#ifndef PIPELINE_SETUP_H
#define PIPELINE_SETUP_H

#include <memory>
#include "./audio_params.h"
#include "./pipeline/pipeline_element.h"

std::shared_ptr<AudioElement<float>> build_pipeline(AudioParams params, bool dumpMidi, Rx::observable<const snd_seq_event_t*> globalMidi);

#endif
