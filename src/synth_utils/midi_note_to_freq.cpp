#include "./midi_note_to_freq.h"

#include <cmath>

float midiNoteToFreq(unsigned char note) {
  return 440 * powf(2, (static_cast<float>(note) - 69) / 12);
}
