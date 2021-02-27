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

class PerVoiceController;
class ThreadedMixerElement;
class AmpElement;

// Wire up synth modules & midi events
class ZSynthController : public MidiNoteListener, public MidiNRPNListener {
  const AudioParams params;
  const uint32_t polyphony;
  PolyphonyPartitioning polyphonyPartitioning;
  float masterAmp;
  float masterOverdrive;

  std::vector<std::shared_ptr<PerVoiceController>> voiceControllers;
  std::shared_ptr<ThreadedMixerElement> mixerElement;
  std::shared_ptr<AmpElement> ampElement;

  std::shared_ptr<AudioElement<float>> _pipeline;

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
  ZSynthController(AudioParams params, uint32_t polyphony, uint32_t numThreads);
  void injectMidi(Rx::observable<const snd_seq_event_t*>);

  std::shared_ptr<AudioElement<float>> pipeline() const;
};

#endif
