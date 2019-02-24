#include <mutex>
#include <vector>
#include <string>
#include <stdio.h>
#include <thread>
#include <errno.h>

#include "./cli.h"

#include <alsa/asoundlib.h>

#include "loops.h"
#include "./pipeline_setup.h"

using namespace std;

snd_seq_t *openMidiDevice(const CLIParams& params) {
  snd_seq_t *midiDevice;
  int portid;

  if (snd_seq_open(&midiDevice, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(midiDevice, params.midiDeviceName.c_str());
  if ((portid = snd_seq_create_simple_port(midiDevice, params.midiDeviceName.c_str(),
          SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
          SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  return midiDevice;
}

static int setHwParams(
  CLIParams& cliParams,
  snd_pcm_t *audioDevice,
  snd_pcm_hw_params_t *params,
  snd_pcm_access_t access
) {
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
  err = snd_pcm_hw_params_set_channels(audioDevice, params, cliParams.channelCount);
  if (err < 0) {
    printf("Channels count (%i) not available for playbacks: %s\n", cliParams.channelCount, snd_strerror(err));
    return err;
  }
  /* set the stream rate */
  rrate = cliParams.rate;
  err = snd_pcm_hw_params_set_rate_near(audioDevice, params, &rrate, 0);
  if (err < 0) {
    printf("Rate %iHz not available for playback: %s\n", cliParams.rate, snd_strerror(err));
    return err;
  }
  if (rrate != cliParams.rate) {
    printf("Rate doesn't match (requested %iHz, got %iHz)\n", cliParams.rate, err);
    return -EINVAL;
  }
  /* set the buffer time */
  unsigned int bufferTimeUs = cliParams.bufferTimeMs * 1000;
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
  cliParams.bufferSize = size;
  /* set the period time */
  unsigned int periodTimeUs = cliParams.periodTimeMs * 1000;
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
  cliParams.periodSize = size;

  printf("Buffer size: %d samples\nPeriod size %d samples\n", (int)cliParams.bufferSize, (int)cliParams.periodSize);
  /* write the parameters to device */
  err = snd_pcm_hw_params(audioDevice, params);
  if (err < 0) {
    printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
    return err;
  }
  return 0;
}

static int setSwParams(const CLIParams& cliParams, snd_pcm_t *audioDevice, snd_pcm_sw_params_t *swparams) {
  int err;
  /* get the current swparams */
  err = snd_pcm_sw_params_current(audioDevice, swparams);
  if (err < 0) {
    printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
    return err;
  }
  /* start the transfer when the buffer is almost full: */
  /* (buffer_size / avail_min) * avail_min */
  err = snd_pcm_sw_params_set_start_threshold(audioDevice, swparams, (cliParams.bufferSize / cliParams.periodSize) * cliParams.periodSize);
  if (err < 0) {
    printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
    return err;
  }
  /* allow the transfer when at least periodSize samples can be processed */
  /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
  err = snd_pcm_sw_params_set_avail_min(audioDevice, swparams, cliParams.periodSize);
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

void encodeToBufferFmt(uint32_t count, float* from, sample_t* to) {
  static const int maxval = (1 << 15) - 1;
  for (uint32_t i = 0; i < count; i++) {
    to[i] = static_cast<sample_t>(from[i] * maxval);
  }
}

void loop(const CLIParams& params, snd_pcm_t* audioDevice, snd_seq_t* midiDevice) {
  mutex lock;
  shared_ptr<MidiAudioElement<float>> pipeline = build_pipeline(params.rate, params.periodSize, params.channelCount);

  vector<float> audioBuffer(params.channelCount * params.periodSize);

  AudioParam audioParam { params.rate, (uint32_t)params.periodSize, params.channelCount };
  thread audioThread(audioLoop, audioDevice, audioParam, [&](sample_t* buffer) {
    {
      lock_guard<mutex> guard(lock);
      pipeline->generate(params.periodSize, audioBuffer.data(), 0, nullptr);
    }
    encodeToBufferFmt(params.periodSize * params.channelCount, audioBuffer.data(), buffer);
  });

  thread midiThread(midiLoop, midiDevice, [&](const snd_seq_event_t* ev) {
    static const unsigned char listenChannel = 0;
    static const unsigned char sustainControlNumber = 64;
    if (ev->data.control.channel != listenChannel) {
      return;
    }
    {
      lock_guard<mutex> guard(lock);
      switch (ev->type) {
        case SND_SEQ_EVENT_CONTROLLER: 
          if (ev->data.control.param == sustainControlNumber) {
            if (ev->data.control.value == 0) {
              pipeline->sustainOffEvent();
            } else {
              pipeline->sustainOnEvent();
            }
          }
          break;
        case SND_SEQ_EVENT_NOTEON:
        case SND_SEQ_EVENT_NOTEOFF: 
          if (ev->data.note.velocity == 0 || ev->type == SND_SEQ_EVENT_NOTEOFF) {
            pipeline->noteOffEvent(ev->data.note.note);
          } else {
            pipeline->noteOnEvent(ev->data.note.note, ev->data.note.velocity);
          }
          break;        
        case SND_SEQ_EVENT_CHANPRESS:
          pipeline->channelPressureEvent(ev->data.control.value);
          break;        
      }
    }
  });

  audioThread.join();
  midiThread.join();
}

int main(int argc, char *argv[]) {
  CLIParams params = parseArgs(argc, argv);

  snd_seq_t *midiDevice = openMidiDevice(params);
  snd_pcm_t *audioDevice;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);

  int err;
  if ((err = snd_pcm_open(&audioDevice, params.device.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    printf("Playback open error: %s\n", snd_strerror(err));
    return 0;
  }

  if ((err = setHwParams(params, audioDevice, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    printf("Setting of hwparams failed: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }
  if ((err = setSwParams(params, audioDevice, swparams)) < 0) {
    printf("Setting of swparams failed: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }

  loop(params, audioDevice, midiDevice);

  snd_pcm_close(audioDevice);
  return 0;
}
