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
class GeneratorElement: public SustainAdapter, public AudioElement<float> {
  uint32_t sampleRateHz;
  uint32_t channelCount;

  std::unordered_map<unsigned char, uint32_t> noteSynths;
  std::unordered_map<uint32_t, std::shared_ptr<NotePipeline>> pipelines;
  std::vector<float> stagingBuffer;

  uint32_t _nextId = 0;
  uint32_t createId();

  void sustainNoteOnEvent(unsigned char note, unsigned char vel) override;
  void sustainNoteOffEvent(unsigned char note) override;

public:
  GeneratorElement(uint32_t sampleRateHz, uint32_t channelCount);

  uint32_t maxInputs() override;
  void generate(uint32_t numSamples, float* out, uint32_t, inputs_t<float>) override;
};

#endif
