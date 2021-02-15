#include "./cli.h"
#include "./metric.h"
#include "./synth_utils/midi_filters.h"
#include "./synth_utils/midi_tap.h"
#include "./z_synth_controller.h"

#include "./alsa/alsa.h"
#include "./alsa/loops.h"

#include <mutex>
#include <thread>

using namespace std;

void loop(AudioParams params, snd_pcm_t* audioDevice, snd_seq_t* midiDevice, CLIParams cliParams) {
  mutex lock;

  Rx::subject<const snd_seq_event_t*> midiSubject;
  auto channelMidi = midiSubject.get_observable() |
    Rx::filter(channelFilter(0));

  Rx::subscription tapSub;
  if (cliParams.dumpMidi) {
    tapSub = midiTapSubscription(channelMidi);
  }

  ZSynthController synth(params);
  synth.injectMidi(channelMidi);

  // Metrics
  const float periodSizeSec = static_cast<float>(params.bufferSampleCount) / params.sampleRateHz;
  const int periodsPerSec = 1 / periodSizeSec;
  const int metricSeconds = 5;
  const int metricsLength = periodsPerSec * metricSeconds;
  Metric audioLatency(metricsLength);
  if (cliParams.dumpMetrics) {
    printf("Dumping audio gen time (%d second sliding window, %d generations, period size %.3f sec)\n", metricSeconds, metricsLength, periodSizeSec);
  }

  uint32_t i = 0;
  thread audioThread(audioLoop, audioDevice, params, [&](float* buffer) {
    lock_guard<mutex> guard(lock);
    {
      TimeMetricRAII time(audioLatency);
      synth.pipeline()->generate(params.bufferSampleCount, buffer, 0, nullptr);
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

  Rx::subscriber<const snd_seq_event_t*> subscriber = midiSubject.get_subscriber();
  thread midiThread(midiLoop, midiDevice, [&](const snd_seq_event_t* ev) {
    // forward message to observers
    lock_guard<mutex> guard(lock);
    subscriber.on_next(ev);
  });

  audioThread.join();
  midiThread.join();
  tapSub.unsubscribe();
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
