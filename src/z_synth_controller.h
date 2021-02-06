#ifndef Z_SYNTH_CONTROLLER_H
#define Z_SYNTH_CONTROLLER_H

#include "./audio_params.h"
#include "./pipeline/pipeline_element.h"
#include "./synth_utils/midi_note_listener.h"
#include "./synth_utils/midi_nrpn_listener.h"
#include "./synth_utils/polyphony_partitioning.h"

#include <cstdio>
#include <memory>
#include <vector>

class ADSRElement;
class AmpElement;
class DistortionElement;
class GeneratorElement;
class LowpassFilterElement;
class MidiPolyphonyAdapter;

// Wire up synth modules & midi events
class ZSynthController : public MidiNoteListener, public MidiNRPNListener {
  static const uint32_t polyphonyCount = 32;
  const AudioParams params;
  PolyphonyPartitioning polyphonyPartitioning;

  // elements
  std::shared_ptr<AmpElement> ampElement;
  std::vector<std::shared_ptr<GeneratorElement>> genElements;
  std::vector<std::shared_ptr<ADSRElement>> adsrElements;
  std::vector<std::shared_ptr<DistortionElement>> distElements;
  std::vector<std::shared_ptr<LowpassFilterElement>> filterElements;
  std::shared_ptr<GeneratorElement> lfoElement;

  // logical element/wiring
  std::shared_ptr<AudioElement<float>> _pipeline;

  std::shared_ptr<AudioElement<float>> makeWiring() const;

  void onNoteOnEvent(unsigned char note, unsigned char vel) override;
  void onNoteOffEvent(unsigned char note) override;
  void onSustainOnEvent() override;
  void onSustainOffEvent() override;
  void onNRPNValueHighChange(
    unsigned char paramHigh,
    unsigned char paramLow,
    unsigned char value
  ) override;

public:
  ZSynthController(AudioParams params);
  void injectMidi(Rx::observable<const snd_seq_event_t*>);

  std::shared_ptr<AudioElement<float>> pipeline() const;
};

#endif
