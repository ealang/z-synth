#include <mutex>
#include <thread>

#include "./rx_include.h"

#include "./alsa/alsa.h"
#include "./alsa/loops.h"

#include "./cli.h"
#include "./pipeline_setup.h"
#include "./synth_utils/midi_filters.h"

using namespace std;

void loop(AudioParams params, snd_pcm_t* audioDevice, snd_seq_t* midiDevice) {
  mutex lock;

  Rx::subject<const snd_seq_event_t*> midiSubject;
  auto midiObservable = midiSubject.get_observable() |
    Rx::filter(channelFilter(0));

  shared_ptr<MidiAudioElement<float>> pipeline = build_pipeline(params);
  pipeline->injectMidi(midiObservable);

  thread audioThread(audioLoop, audioDevice, params, [&](float* buffer) {
    lock_guard<mutex> guard(lock);
    pipeline->generate(params.bufferSampleCount, buffer, 0, nullptr);
  });

  thread midiThread(midiLoop, midiDevice, [&](const snd_seq_event_t* ev) {
    // forward message to observers
    lock_guard<mutex> guard(lock);
    midiSubject.get_subscriber().on_next(ev);
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
