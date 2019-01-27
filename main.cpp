#include <sys/time.h>
#include <cstdint> 
#include <utility>      // std::pair
#include <stdio.h>
#include <unordered_set>
#include <memory>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <unordered_map>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <getopt.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <string>
#include <math.h>

using namespace std;

static string device = "hw:1,0";                     /* playback device */
static snd_pcm_format_t format = SND_PCM_FORMAT_S16;  /* sample format */
static unsigned int rate = 44100;                       /* stream rate */
static unsigned int channels = 1;                       /* count of channels */
static unsigned int buffer_time = 500000;               /* ring buffer length in us */
static unsigned int period_time = 100000;               /* period time in us */
static double freq = 440;                               /* sinusoidal wave frequency in Hz */
static int verbose = 0;                                 /* verbose flag */
static int resample = 1;                                /* enable alsa-lib resampling */
static int period_event = 0;                            /* produce poll event after each period */
static snd_pcm_sframes_t buffer_size;
static snd_pcm_sframes_t period_size;
static snd_output_t *output = NULL;

typedef int16_t sample_t;

uint64_t getTime() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return ((uint64_t)tv.tv_sec * 1000000) + tv.tv_usec;
}

class NoteSynth {
  float sampleRateHz;
  float freqHz;
  float velocity;
  uint64_t timeOn;

  double phase = 0;
  bool off = false;

public:
  NoteSynth(
      unsigned int sampleRateHz,
      float freqHz,
      float velocity,
      uint64_t timeOn
  ): sampleRateHz(sampleRateHz),
     freqHz(freqHz),
     velocity(velocity),
     timeOn(timeOn)
  { }

  ~NoteSynth() {
    printf("delete notesynth\n");
  }

  bool isExhausted() {
    return this->off;
  }

  int generate(uint64_t nSamples, sample_t* buffer) {
    static double max_phase = 2. * M_PI;
    double step = max_phase * this->freqHz / (double)this->sampleRateHz;
    unsigned int maxval = (1 << 15) - 1;
    bool big_endian = snd_pcm_format_big_endian(format) == 1;
    for (int i = 0; i < nSamples; i++) {
      sample_t sample = static_cast<sample_t>(sin(this->phase) * (maxval / 16));
      buffer[i] += sample;
      this->phase += step;
      if (this->phase >= max_phase)
        this->phase -= max_phase;
    }
  }

  void postOffEvent(uint64_t time) {
    this->off = true;
  }
};

float midiNoteToFreq(unsigned char note) {
  return 440 * powf(2, (static_cast<float>(note) - 69) / 12);
}

class NoteMux {
  unordered_map<unsigned char, int> notesMap;
  unordered_map<int, shared_ptr<NoteSynth>> synths;
  uint64_t nextId = 0;

public:

  void noteOnEvent(unsigned char note, unsigned char vel, uint64_t time) {
    if (notesMap.count(note) == 0) {
      auto myId = nextId++;
      auto f = midiNoteToFreq(note);
      auto s = make_shared<NoteSynth>(rate, f, 1, time);
      printf("adding client %d with freq %f\n", (int)myId, f);
      synths.emplace(make_pair(myId, s));
      notesMap[note] = myId;
    }
  }
  void noteOffEvent(unsigned char note, unsigned char vel, uint64_t time) {
    if (notesMap.count(note) > 0) {
      synths[notesMap[note]].get()->postOffEvent(time);
      printf("removing client %d\n", notesMap[note]);
      notesMap.erase(note);
    }
  }

  void generate(sample_t* buffer, int count) {
    // TODO: must delete dead synths
    auto dead = unordered_set<int>();
    memset(buffer, 0, count * sizeof(sample_t));
    for (auto& kv: this->synths) {
      NoteSynth *synth = kv.second.get();
      if (synth->isExhausted()) {
        dead.insert(kv.first);
      } else {
        synth->generate(count, buffer);
      }
    }
    for (auto id: dead) {
      printf("erasing synth %d\n", id);
      this->synths.erase(id);
    }
  }
};

NoteMux noteMux;

snd_seq_t *open_seq() {

  snd_seq_t *seq_handle;
  int portid;

  if (snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, "ALSA Sequencer Demo");
  if ((portid = snd_seq_create_simple_port(seq_handle, "ALSA Sequencer Demo",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  return(seq_handle);
}

void midi_action(snd_seq_t *seq_handle) {

  snd_seq_event_t *ev;

  do {
    snd_seq_event_input(seq_handle, &ev);
    switch (ev->type) {
      /*
      case SND_SEQ_EVENT_CONTROLLER: 
        fprintf(stderr, "Control event on Channel %2d: %5d       \n",
                ev->data.control.channel, ev->data.control.value);
        break;
      case SND_SEQ_EVENT_PITCHBEND:
        fprintf(stderr, "Pitchbender event on Channel %2d: %5d   \n", 
                ev->data.control.channel, ev->data.control.value);
        break;
      */
      case SND_SEQ_EVENT_NOTEON:
        fprintf(stderr, "Note On event on Channel %d: %d (vel %d)\n",
                ev->data.control.channel, ev->data.note.note, ev->data.note.velocity);
        if (ev->data.note.velocity == 0) {
          noteMux.noteOffEvent(ev->data.note.note, ev->data.note.velocity, getTime());
        } else {
          noteMux.noteOnEvent(ev->data.note.note, ev->data.note.velocity, getTime());
        }
        break;        
      case SND_SEQ_EVENT_NOTEOFF: 
        fprintf(stderr, "Note Off event on Channel %2: %d\n",         
                ev->data.control.channel, ev->data.note.note);           
        noteMux.noteOffEvent(ev->data.note.note, ev->data.note.velocity, getTime());
        break;        
      case SND_SEQ_EVENT_CHANPRESS:
        fprintf(stderr, "Aftertouch event on Channel %d: %d\n",         
                ev->data.control.channel, ev->data.control.value);
        break;        
    }
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
}


static void generate_sine(sample_t* samples, int count, double *_phase)
{
  // TODO: thread safety
  noteMux.generate(samples, count);
  /*
  static double max_phase = 2. * M_PI;
  double phase = *_phase;
  double step = max_phase * freq / (double)rate;
  unsigned int maxval = (1 << 15) - 1;
  bool big_endian = snd_pcm_format_big_endian(format) == 1;
  for (int i = 0; i < count; i++) {
    sample_t sample = static_cast<sample_t>(sin(phase) * maxval);
    samples[i] = sample;
    phase += step;
    if (phase >= max_phase)
      phase -= max_phase;
  }
  *_phase = phase;
  */
}

static void help(void)
{
  printf(
      "Usage: pcm [OPTION]... [FILE]...\n"
      "-h,--help      help\n"
      "-D,--device    playback device\n"
      "-r,--rate      stream rate in Hz\n"
      "-b,--buffer    ring buffer size in us\n"
      "-p,--period    period size in us\n"
      "-v,--verbose   show the PCM setup parameters\n"
      "\n");
}

static int set_hwparams(snd_pcm_t *handle,
                        snd_pcm_hw_params_t *params,
                        snd_pcm_access_t access)
{
        unsigned int rrate;
        snd_pcm_uframes_t size;
        int err, dir;
        /* choose all parameters */
        err = snd_pcm_hw_params_any(handle, params);
        if (err < 0) {
                printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
                return err;
        }
        /* set hardware resampling */
        err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
        if (err < 0) {
                printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* set the interleaved read/write format */
        err = snd_pcm_hw_params_set_access(handle, params, access);
        if (err < 0) {
                printf("Access type not available for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* set the sample format */
        err = snd_pcm_hw_params_set_format(handle, params, format);
        if (err < 0) {
                printf("Sample format not available for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* set the count of channels */
        err = snd_pcm_hw_params_set_channels(handle, params, channels);
        if (err < 0) {
                printf("Channels count (%i) not available for playbacks: %s\n", channels, snd_strerror(err));
                return err;
        }
        /* set the stream rate */
        rrate = rate;
        err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
        if (err < 0) {
                printf("Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
                return err;
        }
        if (rrate != rate) {
                printf("Rate doesn't match (requested %iHz, get %iHz)\n", rate, err);
                return -EINVAL;
        }
        /* set the buffer time */
        err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
        if (err < 0) {
                printf("Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(err));
                return err;
        }
        err = snd_pcm_hw_params_get_buffer_size(params, &size);
        if (err < 0) {
                printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
                return err;
        }
        buffer_size = size;
        /* set the period time */
        err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, &dir);
        if (err < 0) {
                printf("Unable to set period time %i for playback: %s\n", period_time, snd_strerror(err));
                return err;
        }
        err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
        if (err < 0) {
                printf("Unable to get period size for playback: %s\n", snd_strerror(err));
                return err;
        }
        period_size = size;

        printf("buffer size %d period size %d\n", buffer_size, period_size);
        /* write the parameters to device */
        err = snd_pcm_hw_params(handle, params);
        if (err < 0) {
                printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
                return err;
        }
        return 0;
}

static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
        int err;
        /* get the current swparams */
        err = snd_pcm_sw_params_current(handle, swparams);
        if (err < 0) {
                printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* start the transfer when the buffer is almost full: */
        /* (buffer_size / avail_min) * avail_min */
        err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
        if (err < 0) {
                printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* allow the transfer when at least period_size samples can be processed */
        /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
        err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_event ? buffer_size : period_size);
        if (err < 0) {
                printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* enable period events when requested */
        if (period_event) {
                err = snd_pcm_sw_params_set_period_event(handle, swparams, 1);
                if (err < 0) {
                        printf("Unable to set period event: %s\n", snd_strerror(err));
                        return err;
                }
        }
        /* write the parameters to the playback device */
        err = snd_pcm_sw_params(handle, swparams);
        if (err < 0) {
                printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
                return err;
        }
        return 0;
}

/*
 *   Transfer method - asynchronous notification
 */
struct async_private_data {
        sample_t* samples;
        double phase;
};
static void async_callback(snd_async_handler_t *ahandler)
{
        snd_pcm_t *handle = snd_async_handler_get_pcm(ahandler);
        struct async_private_data *data = (async_private_data *)snd_async_handler_get_callback_private(ahandler);

        sample_t *samples = data->samples;
        snd_pcm_sframes_t avail;
        int err;
        
        avail = snd_pcm_avail_update(handle);
        while (avail >= period_size) {
                generate_sine(data->samples, period_size, &data->phase);
                err = snd_pcm_writei(handle, samples, period_size);
                if (err < 0) {
                        printf("Write error: %s\n", snd_strerror(err));
                        // exit(EXIT_FAILURE);
                }
                if (err != period_size) {
                        printf("Write error: written %i expected %li\n", err, period_size);
                        exit(EXIT_FAILURE);
                }
                avail = snd_pcm_avail_update(handle);
        }
}
static int async_loop(snd_pcm_t* handle,
                      sample_t* samples) {
        async_private_data *data = new async_private_data { samples, 0 };
        snd_async_handler_t *ahandler;
        int err, count;
        err = snd_async_add_pcm_handler(&ahandler, handle, async_callback, data);
        if (err < 0) {
                printf("Unable to register async handler\n");
                exit(EXIT_FAILURE);
        }
        for (count = 0; count < 2; count++) {
                generate_sine(samples, period_size, &data->phase);
                err = snd_pcm_writei(handle, samples, period_size);
                if (err < 0) {
                        printf("Initial write error: %s\n", snd_strerror(err));
                        exit(EXIT_FAILURE);
                }
                if (err != period_size) {
                        printf("Initial write error: written %i expected %li\n", err, period_size);
                        exit(EXIT_FAILURE);
                }
        }
        if (snd_pcm_state(handle) == SND_PCM_STATE_PREPARED) {
                err = snd_pcm_start(handle);
                if (err < 0) {
                        printf("Start error: %s\n", snd_strerror(err));
                        exit(EXIT_FAILURE);
                }
        }
}


int main(int argc, char *argv[])
{
  struct option long_option[] =
  {
    {"help", 0, NULL, 'h'},
    {"device", 1, NULL, 'D'},
    {"rate", 1, NULL, 'r'},
    {"buffer", 1, NULL, 'b'},
    {"period", 1, NULL, 'p'},
    {"verbose", 1, NULL, 'v'},
    {NULL, 0, NULL, 0},
  };
  snd_pcm_t *handle;
  int err, morehelp;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  int method = 0;
  sample_t *samples;
  unsigned int chn;
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
        buffer_time = atoi(optarg);
        buffer_time = buffer_time < 1000 ? 1000 : buffer_time;
        buffer_time = buffer_time > 1000000 ? 1000000 : buffer_time;
        break;
      case 'p':
        period_time = atoi(optarg);
        period_time = period_time < 1000 ? 1000 : period_time;
        period_time = period_time > 1000000 ? 1000000 : period_time;
        break;
      case 'v':
        verbose = 1;
        break;
    }
  }
  if (morehelp) {
    help();
    return 0;
  }
  err = snd_output_stdio_attach(&output, stdout, 0);
  if (err < 0) {
    printf("Output failed: %s\n", snd_strerror(err));
    return 0;
  }
  printf("Playback device is %s\n", device.c_str());
  printf("Stream parameters are %iHz, %s, %i channels\n", rate, snd_pcm_format_name(format), channels);
  printf("Sine wave rate is %.4fHz\n", freq);
  if ((err = snd_pcm_open(&handle, device.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    printf("Playback open error: %s\n", snd_strerror(err));
    return 0;
  }

  if ((err = set_hwparams(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    printf("Setting of hwparams failed: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }
  if ((err = set_swparams(handle, swparams)) < 0) {
    printf("Setting of swparams failed: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }
  if (verbose > 0)
    snd_pcm_dump(handle, output);
  printf("allocated %d\n", period_size);
  samples = new sample_t[period_size];
  err = async_loop(handle, samples);
  if (err < 0)
    printf("Transfer failed: %s\n", snd_strerror(err));



  snd_seq_t *seq_handle;
  int npfd;
  struct pollfd *pfd;
    
  seq_handle = open_seq();
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
  while (1) {
    if (poll(pfd, npfd, 1000000) > 0) {
      midi_action(seq_handle);
    }  
  }




  delete[] samples;
  snd_pcm_close(handle);
  return 0;
}
