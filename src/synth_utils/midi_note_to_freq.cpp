#include "./midi_note_to_freq.h"

#include <cmath>

float midiNoteToFreq(float note) {
  return 440 * powf(2, (note - 69) / 12);
}
