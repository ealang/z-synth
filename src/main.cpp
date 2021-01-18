#include <mutex>
#include <thread>

#include "./rx_include.h"

#include "./alsa/alsa.h"
#include "./alsa/loops.h"

#include "./cli.h"
#include "./elements/midi_tap_element.h"
#include "./elements/polyphony_element.h"
#include "./metric.h"
#include "./pipeline_setup.h"
#include "./synth_utils/midi_filters.h"

using namespace std;

void loop(AudioParams params, snd_pcm_t* audioDevice, snd_seq_t* midiDevice, CLIParams cliParams) {
  mutex lock;

  Rx::subject<const snd_seq_event_t*> midiSubject;
  auto globalMidi = midiSubject.get_observable() |
    Rx::filter(channelFilter(0));

  MidiTapElement tap;
  if (cliParams.dumpMidi) {
    tap.injectMidi(globalMidi);
  }

  const uint32_t polyphonyCount = 1;
  PolyphonyElement polyphony(polyphonyCount);
  polyphony.injectMidi(globalMidi);

  int periodsPerSec = params.sampleRateHz / params.bufferSampleCount;
  int metricSeconds = 10;
  Metric audioLatency(periodsPerSec * metricSeconds);
  if (cliParams.dumpMetrics) {
    printf("Dumping audio gen time (%d second sliding window, %d periods)\n", metricSeconds, metricSeconds * periodsPerSec);
  }

  shared_ptr<AudioElement<float>> pipeline = build_pipeline(params, globalMidi, polyphony);

  uint32_t i = 0;

  thread audioThread(audioLoop, audioDevice, params, [&](float* buffer) {
    lock_guard<mutex> guard(lock);
    {
      TimeMetricRAII time(audioLatency);
      pipeline->generate(params.bufferSampleCount, buffer, 0, nullptr);
    }

    i += 1;
    if (cliParams.dumpMetrics && (i % periodsPerSec) == 0) {
      auto report = audioLatency.report();
      printf("Audio gen time: max=%.3fmS p99=%.3fmS p75=%.3fmS\n",
        report.max * 1000,
        report.p99 * 1000,
        report.p75 * 1000
      );
    }
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

  loop(audioParams, audioDevice, midiDevice, cliParams);

  return 0;
}
