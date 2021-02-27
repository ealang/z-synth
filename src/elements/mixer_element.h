#ifndef MIXER_ELEMENT_H
#define MIXER_ELEMENT_H

#include "../pipeline/pipeline_element.h"

#include <cstdint>
#include <vector>

/**
 * Weighted combination of other signals.
 */
class MixerElement: public AudioElement<float> {
  std::vector<float> _weights;
public:
  MixerElement(uint32_t n);
  MixerElement(uint32_t n, float weight);
  MixerElement(std::vector<float> weights);

  uint32_t maxInputs() const override;
  uint32_t inputPortNumber(uint32_t i) const;

  void setWeight(uint32_t i, float weight);
  void setWeights(std::vector<float> weights);

  void generate(uint32_t numSamples, float* out, uint32_t numInputs, inputs_t<float> inputs) override;
};

#endif
