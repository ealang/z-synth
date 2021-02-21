#include <getopt.h>
#include "./cli.h"

static void help(const CLIParams& params) {
  printf(
    "Usage: midisynth [OPTION]... [FILE]...\n"
    "-h,--help         help\n"
    "-D,--device       playback device (default: %s)\n"
    "-r,--rate         stream rate in Hz (default: %d)\n"
    "-b,--buffer       buffer size in ms (default: %d)\n"
    "-p,--period       period size in ms (default: %d)\n"
    "-c,--channels     number of channels (default: %d)\n"
    "-t,--threads      number of threads (default: num cores)\n"
    "-d,--dump-midi    print midi messages\n"
    "-m,--dump-metrics print performance metrics\n"
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
    {"threads", 1, NULL, 't'},
    {"dump-midi", 0, NULL, 'd'},
    {"dump-metrics", 0, NULL, 'm'},
    {NULL, 0, NULL, 0},
  };
  bool showHelp = false;
  while (1) {
    int c;
    if ((c = getopt_long(argc, argv, "hD:r:b:p:c:t:dm", long_option, NULL)) < 0)
      break;
    switch (c) {
      case 'h':
        showHelp = true;
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
      case 't':
        params.threadCount = atoi(optarg);
        break;
      case 'd':
        params.dumpMidi = true;
        break;
      case 'm':
        params.dumpMetrics = true;
    }
  }
  if (showHelp) {
    help(params);
    exit(EXIT_SUCCESS);
  }

  return params;
}
