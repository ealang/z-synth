#ifndef CLI_H
#define CLI_H

#include <cstdint>
#include <string>
#include <alsa/asoundlib.h>

struct CLIParams {
  std::string midiDeviceName = "z-synth";
  std::string device = "hw:0,0";
  uint32_t rate = 44100;
  uint32_t bufferTimeMs = 10;
  uint32_t periodTimeMs = 5;
  uint32_t channelCount = 2;
  uint32_t threadCount = 0;
  bool dumpMidi = false;
  bool dumpMetrics = false;
};

CLIParams parseArgs(int argc, char *argv[]);

#endif
