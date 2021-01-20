#ifndef REPLICA_SYNTH_H
#define REPLICA_SYNTH_H

#include "./audio_params.h"
#include "./pipeline/pipeline_element.h"

#include <memory>

class MidiPolyphonyAdapter;

// A configuration modeled after a classic synth made polyphonic.
class ReplicaSynth
{
  std::shared_ptr<AudioElement<float>> p;
  static const uint32_t polyphonyCount = 8;
  std::shared_ptr<MidiPolyphonyAdapter> polyphony;

  public:
    ReplicaSynth(AudioParams params, Rx::observable<const snd_seq_event_t*> globalMidi);

    std::shared_ptr<AudioElement<float>> &pipeline();
};

#endif
