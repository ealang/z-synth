#include <mutex>
#include <thread>

#include "./alsa.h"
#include "./cli.h"
#include "./loops.h"
#include "./pipeline_setup.h"

using namespace std;

void loop(AudioParams params, snd_pcm_t* audioDevice, snd_seq_t* midiDevice) {
  mutex lock;
  shared_ptr<MidiAudioElement<float>> pipeline = build_pipeline(params.sampleRateHz, params.bufferSampleCount, params.channelCount);

  thread audioThread(audioLoop, audioDevice, params, [&](float* buffer) {
    lock_guard<mutex> guard(lock);
    pipeline->generate(params.bufferSampleCount, buffer, 0, nullptr);
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
  CLIParams cliParams = parseArgs(argc, argv);

  snd_seq_t* midiDevice = openMidiDevice(cliParams);

  auto audioInfo = openAudioDevice(cliParams);
  snd_pcm_t* audioDevice = audioInfo.first;
  AudioParams audioParams = audioInfo.second;

  loop(audioParams, audioDevice, midiDevice);

  return 0;
}
