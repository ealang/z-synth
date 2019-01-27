#include <string>
#include <stdio.h>
#include <thread>
#include <errno.h>
#include <getopt.h>

#include <alsa/asoundlib.h>

#include "midi_mux.h"
#include "loops.h"

using namespace std;

const static string midiDeviceName = "z-synth";
static string device = "hw:0,0";          /* playback device */
static unsigned int rate = 44100;         /* stream rate */
static unsigned int bufferTimeMs = 20;  /* ring buffer length in ms */
static unsigned int periodTimeMs = 10;   /* period time in ms */
static snd_pcm_sframes_t bufferSize;
static snd_pcm_sframes_t periodSize;

snd_seq_t *openMidiDevice() {
  snd_seq_t *midiDevice;
  int portid;

  if (snd_seq_open(&midiDevice, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(midiDevice, midiDeviceName.c_str());
  if ((portid = snd_seq_create_simple_port(midiDevice, midiDeviceName.c_str(),
          SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
          SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  return midiDevice;
}

static void help(void) {
  printf(
    "Usage: midisynth [OPTION]... [FILE]...\n"
    "-h,--help      help\n"
    "-D,--device    playback device (default: %s)\n"
    "-r,--rate      stream rate in Hz (default: %d)\n"
    "-b,--buffer    buffer size in ms (default: %d)\n"
    "-p,--period    period size in ms (default: %d)\n"
    "\n"
  , device.c_str(), rate, bufferTimeMs, periodTimeMs);
}

static int setHwParams(snd_pcm_t *audioDevice,
    snd_pcm_hw_params_t *params,
    snd_pcm_access_t access) {
  unsigned int rrate;
  snd_pcm_uframes_t size;
  int err, dir;
  /* choose all parameters */
  err = snd_pcm_hw_params_any(audioDevice, params);
  if (err < 0) {
    printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
    return err;
  }
  /* set hardware resampling */
  int resample = 1;                  /* enable alsa-lib resampling */
  err = snd_pcm_hw_params_set_rate_resample(audioDevice, params, resample);
  if (err < 0) {
    printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
    return err;
  }
  /* set the interleaved read/write format */
  err = snd_pcm_hw_params_set_access(audioDevice, params, access);
  if (err < 0) {
    printf("Access type not available for playback: %s\n", snd_strerror(err));
    return err;
  }
  /* set the sample format */
  err = snd_pcm_hw_params_set_format(audioDevice, params, SND_PCM_FORMAT_S16);
  if (err < 0) {
    printf("Sample format not available for playback: %s\n", snd_strerror(err));
    return err;
  }
  /* set the count of channels */
  unsigned int channels = 1;
  err = snd_pcm_hw_params_set_channels(audioDevice, params, channels);
  if (err < 0) {
    printf("Channels count (%i) not available for playbacks: %s\n", channels, snd_strerror(err));
    return err;
  }
  /* set the stream rate */
  rrate = rate;
  err = snd_pcm_hw_params_set_rate_near(audioDevice, params, &rrate, 0);
  if (err < 0) {
    printf("Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
    return err;
  }
  if (rrate != rate) {
    printf("Rate doesn't match (requested %iHz, got %iHz)\n", rate, err);
    return -EINVAL;
  }
  /* set the buffer time */
  unsigned int bufferTimeUs = bufferTimeMs * 1000;
  err = snd_pcm_hw_params_set_buffer_time_near(audioDevice, params, &bufferTimeUs, &dir);
  if (err < 0) {
    printf("Unable to set buffer time %i for playback: %s\n", bufferTimeUs, snd_strerror(err));
    return err;
  }
  err = snd_pcm_hw_params_get_buffer_size(params, &size);
  if (err < 0) {
    printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
    return err;
  }
  bufferSize = size;
  /* set the period time */
  unsigned int periodTimeUs = periodTimeMs * 1000;
  err = snd_pcm_hw_params_set_period_time_near(audioDevice, params, &periodTimeUs, &dir);
  if (err < 0) {
    printf("Unable to set period time %i for playback: %s\n", periodTimeUs, snd_strerror(err));
    return err;
  }
  err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
  if (err < 0) {
    printf("Unable to get period size for playback: %s\n", snd_strerror(err));
    return err;
  }
  periodSize = size;

  printf("Buffer size: %d samples\nPeriod size %d samples\n", (int)bufferSize, (int)periodSize);
  /* write the parameters to device */
  err = snd_pcm_hw_params(audioDevice, params);
  if (err < 0) {
    printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
    return err;
  }
  return 0;
}

static int setSwParams(snd_pcm_t *audioDevice, snd_pcm_sw_params_t *swparams) {
  int err;
  /* get the current swparams */
  err = snd_pcm_sw_params_current(audioDevice, swparams);
  if (err < 0) {
    printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
    return err;
  }
  /* start the transfer when the buffer is almost full: */
  /* (buffer_size / avail_min) * avail_min */
  err = snd_pcm_sw_params_set_start_threshold(audioDevice, swparams, (bufferSize / periodSize) * periodSize);
  if (err < 0) {
    printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
    return err;
  }
  /* allow the transfer when at least periodSize samples can be processed */
  /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
  err = snd_pcm_sw_params_set_avail_min(audioDevice, swparams, periodSize);
  if (err < 0) {
    printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
    return err;
  }
  /* write the parameters to the playback device */
  err = snd_pcm_sw_params(audioDevice, swparams);
  if (err < 0) {
    printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
    return err;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  struct option long_option[] =
  {
    {"help", 0, NULL, 'h'},
    {"device", 1, NULL, 'D'},
    {"rate", 1, NULL, 'r'},
    {"buffer", 1, NULL, 'b'},
    {"period", 1, NULL, 'p'},
    {NULL, 0, NULL, 0},
  };
  snd_pcm_t *audioDevice;
  int err, morehelp;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);
  morehelp = 0;
  while (1) {
    int c;
    if ((c = getopt_long(argc, argv, "hD:r:c:f:b:p:m:o:vne", long_option, NULL)) < 0)
      break;
    switch (c) {
      case 'h':
        morehelp++;
        break;
      case 'D':
        device = strdup(optarg);
        break;
      case 'r':
        rate = atoi(optarg);
        rate = rate < 4000 ? 4000 : rate;
        rate = rate > 196000 ? 196000 : rate;
        break;
      case 'b':
        bufferTimeMs = atoi(optarg);
        break;
      case 'p':
        periodTimeMs = atoi(optarg);
        break;
    }
  }
  if (morehelp) {
    help();
    return 0;
  }

  if ((err = snd_pcm_open(&audioDevice, device.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    printf("Playback open error: %s\n", snd_strerror(err));
    return 0;
  }

  if ((err = setHwParams(audioDevice, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    printf("Setting of hwparams failed: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }
  if ((err = setSwParams(audioDevice, swparams)) < 0) {
    printf("Setting of swparams failed: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }

  MidiMux midiMux(rate);

  snd_seq_t *midiDevice = openMidiDevice();

  thread audioThread(audioLoop, audioDevice, &midiMux, (int)periodSize);
  thread midiThread(midiLoop, midiDevice, &midiMux);

  audioThread.join();
  midiThread.join();

  snd_pcm_close(audioDevice);
  return 0;
}
