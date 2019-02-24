#include <vector>
#include "./loops.h"

using namespace std;

int midiLoop(snd_seq_t *const midiDevice, function<void(const snd_seq_event_t*)> onEvent) {
  while (1) {
    pollMidiEvents(midiDevice, onEvent);
  }
}

static void encodeToBufferFmt(uint32_t count, float* from, sample_t* to) {
  static const int maxval = (1 << 15) - 1;
  for (uint32_t i = 0; i < count; i++) {
    to[i] = static_cast<sample_t>(from[i] * maxval);
  }
}

int audioLoop(
  snd_pcm_t* audioDevice,
  AudioParams audioParam,
  std::function<void(float*)> onSample
) {
  uint32_t totalSamples = audioParam.bufferSampleCount * audioParam.channelCount;

  vector<float> userBuffer(totalSamples);
  vector<sample_t> hwBuffer(totalSamples);

  float* userData = userBuffer.data();
  sample_t* hwData = hwBuffer.data();

  while (1) {
    onSample(userData);
    encodeToBufferFmt(totalSamples, userData, hwData);
    writeAudioFrames(audioDevice, audioParam.bufferSampleCount, hwData);
  }
}
