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
  snd_pcm_sframes_t bufferSize;
  snd_pcm_sframes_t periodSize;
};

CLIParams parseArgs(int argc, char *argv[]);

#endif
