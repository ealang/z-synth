#include <alsa/asoundlib.h>

#include <memory>
#include "./loops.h"

using namespace std;

typedef int16_t sample_t;
unsigned int maxval = (1 << 15) - 1;

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

void encodeToBufferFmt(uint32_t count, float* from, sample_t* to) {
  for (uint32_t i = 0; i < count; i++) {
    to[i] = static_cast<sample_t>(from[i] * maxval);
  }
}

int audioLoop(snd_pcm_t *const audioDevice, shared_ptr<MidiAudioElement<float>> pipeline, AudioParam audioParam) {
  uint32_t samplesPerGen = audioParam.bufferSampleCount * audioParam.channelCount;
  auto samplesU = new sample_t[samplesPerGen];
  float* samplesF = new float[samplesPerGen];
  const float* inSamplesF[] = { samplesF };

  sample_t* ptr;
  int err, cptr;
  while (1) {
    memset(samplesF, 0, sizeof(float) * samplesPerGen);
    pipeline->generate(audioParam.bufferSampleCount, samplesF, inSamplesF);
    encodeToBufferFmt(samplesPerGen, samplesF, samplesU);

    ptr = samplesU;
    cptr = audioParam.bufferSampleCount;
    while (cptr > 0) {
      err = snd_pcm_writei(audioDevice, ptr, cptr);
      if (err == -EAGAIN)
        continue;
      if (err < 0) {
        if (xrunRecovery(audioDevice, err) < 0) {
          printf("Write error: %s\n", snd_strerror(err));
          exit(EXIT_FAILURE);
        }
        break;  /* skip one period */
      }
      ptr += err;
      cptr -= err;
    }
  }

  delete[] samplesU;
  delete[] samplesF;
}

static void handleMidiEvent(shared_ptr<MidiAudioElement<float>> pipeline, snd_seq_t *const midiDevice) {
  static const unsigned char listenChannel = 0;
  static const unsigned char sustainControlNumber = 64;
  snd_seq_event_t *ev;

  do {
    snd_seq_event_input(midiDevice, &ev);
    if (ev->data.control.channel != listenChannel) {
      continue;
    }

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
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(midiDevice, 0) > 0);
}

int midiLoop(snd_seq_t *const midiDevice, shared_ptr<MidiAudioElement<float>> pipeline) {
  int npfd;
  struct pollfd *pfd;

  npfd = snd_seq_poll_descriptors_count(midiDevice, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(midiDevice, pfd, npfd, POLLIN);
  while (1) {
    if (poll(pfd, npfd, 1000000) > 0) {
      handleMidiEvent(pipeline, midiDevice);
    }  
  }
}
