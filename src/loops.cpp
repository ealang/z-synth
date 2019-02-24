#include "./loops.h"

using namespace std;

static int xrunRecovery(snd_pcm_t *const audioDevice, int err) {
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

int audioLoop(
  snd_pcm_t* audioDevice,
  AudioParam audioParam,
  std::function<void(sample_t*)> onSample
) {
  uint32_t totalSamples = audioParam.bufferSampleCount * audioParam.channelCount;
  sample_t* samples = new sample_t[totalSamples];

  while (1) {
    onSample(samples);

    sample_t* ptr = samples;
    int remaining = audioParam.bufferSampleCount;
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

  delete[] samples;
}

int midiLoop(snd_seq_t *const midiDevice, function<void(const snd_seq_event_t*)> onEvent) {
  int npfd = snd_seq_poll_descriptors_count(midiDevice, POLLIN);
  pollfd* pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(midiDevice, pfd, npfd, POLLIN);
  while (1) {
    if (poll(pfd, npfd, 1000000) > 0) {
      snd_seq_event_t *ev;
      do {
        snd_seq_event_input(midiDevice, &ev);
        onEvent(ev);
        snd_seq_free_event(ev);
      } while (snd_seq_event_input_pending(midiDevice, 0) > 0);
    }  
  }
}
