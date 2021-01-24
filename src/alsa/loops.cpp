#include <vector>
#include "./loops.h"

using namespace std;

int midiLoop(snd_seq_t *const midiDevice, function<void(const snd_seq_event_t*)> onEvent) {
  while (1) {
    pollMidiEvents(midiDevice, onEvent);
  }
}

static void encodeToBufferFmt(sample_t* to, uint32_t count, const float* from, uint32_t numChannels) {
  static const int maxval = (1 << 15) - 1;
  for (uint32_t i = 0; i < count; ++i) {
    sample_t val = *(from++) * maxval;
    for (uint32_t c = 0; c < numChannels; ++c) {
      *(to++) = val;
    }
  }
}

int audioLoop(
  snd_pcm_t* audioDevice,
  AudioParams audioParam,
  std::function<void(float*)> onSample
) {
  vector<float> userBuffer(audioParam.bufferSampleCount);
  vector<sample_t> hwBuffer(audioParam.bufferSampleCount * audioParam.channelCount);

  float* userData = userBuffer.data();
  sample_t* hwData = hwBuffer.data();

  while (1) {
    onSample(userData);
    encodeToBufferFmt(hwData, audioParam.bufferSampleCount, userData, audioParam.channelCount);
    writeAudioFrames(audioDevice, audioParam.bufferSampleCount, hwData);
  }
}
