#include "./switch_element.h"

#include <string.h>

SwitchElement::SwitchElement(uint32_t channelCount):
  channelCount(channelCount) {
}

uint32_t SwitchElement::maxInputs() {
  return 0;
}

void SwitchElement::generate(uint32_t numSamples, float* out, uint32_t, inputs_t<float>) {
  if (!core)
  {
    memset(out, 0, sizeof(float) * numSamples * channelCount);
    return;
  }
  core->generate(numSamples, out, 0, nullptr);
};

void SwitchElement::setCore(std::shared_ptr<AudioElement<float>> new_core)
{
  core = new_core;
}
