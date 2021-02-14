#include "./midi_note_to_freq.h"

#include <cmath>

float midiNoteToFreq(unsigned char note) {
  return midiNoteToFreq(static_cast<float>(note));
}

float midiNoteToFreq(float note) {
  return 440 * powf(2, (note - 69) / 12);
}
