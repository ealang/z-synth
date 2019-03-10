#include <getopt.h>
#include "./cli.h"

static void help(const CLIParams& params) {
  printf(
    "Usage: midisynth [OPTION]... [FILE]...\n"
    "-h,--help       help\n"
    "-D,--device     playback device (default: %s)\n"
    "-r,--rate       stream rate in Hz (default: %d)\n"
    "-b,--buffer     buffer size in ms (default: %d)\n"
    "-p,--period     period size in ms (default: %d)\n"
    "-c,--channels   number of channels (default: %d)\n"
    "-m,--dump-midi  print midi messages\n"
    "\n",
    params.device.c_str(),
    params.rate,
    params.bufferTimeMs,
    params.periodTimeMs,
    params.channelCount
  );
}

CLIParams parseArgs(int argc, char *argv[]) {
  CLIParams params;
  struct option long_option[] =
  {
    {"help", 0, NULL, 'h'},
    {"device", 1, NULL, 'D'},
    {"rate", 1, NULL, 'r'},
    {"buffer", 1, NULL, 'b'},
    {"period", 1, NULL, 'p'},
    {"channels", 1, NULL, 'c'},
    {"dump-midi", 0, NULL, 'm'},
    {NULL, 0, NULL, 0},
  };
  int morehelp = 0;
  while (1) {
    int c;
    if ((c = getopt_long(argc, argv, "hD:r:c:f:b:p:m:o:vne", long_option, NULL)) < 0)
      break;
    switch (c) {
      case 'h':
        morehelp++;
        break;
      case 'D':
        params.device = strdup(optarg);
        break;
      case 'r':
        params.rate = atoi(optarg);
        break;
      case 'b':
        params.bufferTimeMs = atoi(optarg);
        break;
      case 'p':
        params.periodTimeMs = atoi(optarg);
        break;
      case 'c':
        params.channelCount = atoi(optarg);
        break;
      case 'm':
        params.dumpMidi = true;
    }
  }
  if (morehelp) {
    help(params);
    exit(EXIT_SUCCESS);
  }

  return params;
}
