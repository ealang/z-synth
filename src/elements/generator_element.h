#ifndef GENERATOR_ELEMENT_H
#define GENERATOR_ELEMENT_H

#include <cstdint> 
#include <memory>
#include <unordered_map>
#include <vector>

#include "../pipeline/pipeline_element.h"
#include "../synth_utils/sustain_adapter.h"

class NotePipeline;

/* Midi aware element that can play multiple notes simultaneously using
 * internal per-note pipelines. Yo dawg, I heard you like pipelines...
 */
class GeneratorElement: public SustainAdapter, public MidiAudioElement<float> {
  uint32_t sampleRateHz;
  uint32_t channelCount;
  float attackTimeMs = 10;
  float releaseTimeMs = 100;
  float decayTimeMs = 30;
  float sustainAmp = 0.5;
  std::vector<Rx::subscription> subs;

  std::unordered_map<unsigned char, uint32_t> noteSynths;
  std::unordered_map<uint32_t, std::shared_ptr<NotePipeline>> pipelines;
  std::vector<float> stagingBuffer;

  uint32_t _nextId = 0;
  uint32_t createId();

  void sustainNoteOnEvent(unsigned char note, unsigned char vel) override;
  void sustainNoteOffEvent(unsigned char note) override;

public:
  GeneratorElement(uint32_t sampleRateHz, uint32_t channelCount);
  ~GeneratorElement();

  void injectMidi(Rx::observable<const snd_seq_event_t*>) override;

  uint32_t maxInputs() override;
  void generate(uint32_t numSamples, float* out, uint32_t, inputs_t<float>) override;
};

#endif
