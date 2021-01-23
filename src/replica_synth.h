#ifndef REPLICA_SYNTH_H
#define REPLICA_SYNTH_H

#include "./audio_params.h"
#include "./pipeline/pipeline_element.h"
#include "./synth_utils/midi_note_listener.h"
#include "./synth_utils/polyphony_partitioning.h"

#include <cstdio>
#include <memory>
#include <vector>

class MidiPolyphonyAdapter;
class AmpElement;
class SquareElement;
class DistortionElement;

// A configuration modeled after a classic synth made polyphonic.
class ReplicaSynth : public MidiNoteListener {
  static const uint32_t polyphonyCount = 8;
  const AudioParams params;
  PolyphonyPartitioning polyphonyPartitioning;

  // elements
  std::shared_ptr<AmpElement> ampElement;
  std::vector<std::shared_ptr<SquareElement>> squareElems;
  std::shared_ptr<DistortionElement> distElement;

  // logical element/wiring
  std::shared_ptr<AudioElement<float>> _pipeline;

  std::shared_ptr<AudioElement<float>> makeWiring1() const;
  std::shared_ptr<AudioElement<float>> makeWiring2() const;

  void onNoteOnEvent(unsigned char note, unsigned char vel) override;
  void onNoteOffEvent(unsigned char note) override;
  void onSustainOnEvent() override;
  void onSustainOffEvent() override;

public:
  ReplicaSynth(AudioParams params);
  void injectMidi(Rx::observable<const snd_seq_event_t*>);

  std::shared_ptr<AudioElement<float>> pipeline() const;
};

#endif
