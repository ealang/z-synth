#ifndef REPLICA_SYNTH_H
#define REPLICA_SYNTH_H

#include "./audio_params.h"
#include "./pipeline/pipeline_element.h"

#include <cstdio>
#include <memory>
#include <vector>

class MidiPolyphonyAdapter;
class SwitchElement;
class AmpElement;

// A configuration modeled after a classic synth made polyphonic.
class ReplicaSynth : public MidiListener {
  static const uint32_t polyphonyCount = 8;
  const AudioParams params;

  std::shared_ptr<MidiPolyphonyAdapter> polyphony;
  std::vector<std::shared_ptr<SwitchElement>> switchElements;
  std::shared_ptr<AmpElement> ampElement;

  std::shared_ptr<AudioElement<float>> _pipeline;

  void initPipeline();

  public:
    ReplicaSynth(AudioParams params);
    void injectMidi(Rx::observable<const snd_seq_event_t*>) override;

    std::shared_ptr<AudioElement<float>> pipeline() const;
};

#endif
