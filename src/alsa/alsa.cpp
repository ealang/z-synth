#include "./alsa.h"
#include "./cli.h"

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

void pollMidiEvents(snd_seq_t *const midiDevice, function<void(const snd_seq_event_t*)> onEvent) {
  int npfd = snd_seq_poll_descriptors_count(midiDevice, POLLIN);
  pollfd* pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(midiDevice, pfd, npfd, POLLIN);
  if (poll(pfd, npfd, 1000000) > 0) {
    snd_seq_event_t *ev;
    do {
      snd_seq_event_input(midiDevice, &ev);
      onEvent(ev);
      snd_seq_free_event(ev);
    } while (snd_seq_event_input_pending(midiDevice, 0) > 0);
  }  
}

static int setHwParams(
  const CLIParams& cliParams,
  snd_pcm_t *audioDevice,
  snd_pcm_hw_params_t *params,
  snd_pcm_access_t access,
  uint32_t& periodSizeOut,
  uint32_t& bufferSizeOut
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
  bufferSizeOut = size;
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
  periodSizeOut = size;

  /* write the parameters to device */
  err = snd_pcm_hw_params(audioDevice, params);
  if (err < 0) {
    printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
    return err;
  }

  return 0;
}

static int setSwParams(
  snd_pcm_t *audioDevice,
  snd_pcm_sw_params_t *swparams,
  uint32_t periodSize,
  uint32_t bufferSize
) {
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

pair<snd_pcm_t*, AudioParams> openAudioDevice(const CLIParams& cliParams) {
  snd_pcm_t *audioDevice;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);

  int err;
  if ((err = snd_pcm_open(&audioDevice, cliParams.device.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    printf("Playback open error: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }

  uint32_t periodSize, bufferSize;
  if ((err = setHwParams(cliParams, audioDevice, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED, periodSize, bufferSize)) < 0) { printf("Setting of hwparams failed: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }
  if ((err = setSwParams(audioDevice, swparams, periodSize, bufferSize)) < 0) {
    printf("Setting of swparams failed: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }

  printf(
    "Buffer size: %d frames\n"
    "Period size %d frames\n",
    (int)bufferSize,
    (int)periodSize
  );

  return make_pair(
    audioDevice,
    AudioParams { cliParams.rate, periodSize, cliParams.channelCount }
  );
}

static int xrunRecovery(snd_pcm_t* audioDevice, int err) {
  if (err == -EPIPE) {    /* under-run */
    err = snd_pcm_prepare(audioDevice);
    if (err < 0)
      printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
    return 0;
  } else if (err == -ESTRPIPE) {
    while ((err = snd_pcm_resume(audioDevice)) == -EAGAIN)
      sleep(1);       /* wait until the suspend flag is released */
    if (err < 0) {
      err = snd_pcm_prepare(audioDevice);
      if (err < 0)
        printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
    }
    return 0;
  }
  return err;
}

void writeAudioFrames(snd_pcm_t* audioDevice, uint32_t numFrames, sample_t *data) {
  sample_t* ptr = data;
  int remaining = numFrames;
  while (remaining > 0) {
    int err = snd_pcm_writei(audioDevice, ptr, remaining);
    if (err == -EAGAIN) {
      continue;
    }
    if (err < 0) {
      if (xrunRecovery(audioDevice, err) < 0) {
        printf("Write error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
      }
      break;  /* skip one period */
    }
    ptr += err;
    remaining -= err;
  }
}
