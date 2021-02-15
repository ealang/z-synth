#include <algorithm>
#include "./metric.h"

using namespace std;

TimeMetricRAII::TimeMetricRAII(Metric& metric): metric(metric) {
  start = clock();
}

TimeMetricRAII::~TimeMetricRAII() {
  float deltaUs = (float)(clock() - start) / CLOCKS_PER_SEC;
  metric.record(deltaUs);
}

std::function<float()> stopWatchSeconds() {
  clock_t start = clock();
  return [start]() {
    return (float)(clock() - start) / CLOCKS_PER_SEC;
  };
}

Metric::Metric(uint32_t length): values(length) {}

void Metric::record(float value) {
  values[i] = value;
  i = (i + 1) % values.size();
}

MetricReport Metric::report() const {
  vector<float> sorted(values);
  sort(sorted.begin(), sorted.end());

  uint32_t n = sorted.size();
  return MetricReport {
    sorted[n - 1],
    sorted[static_cast<uint32_t>(n * 0.99)],
    sorted[static_cast<uint32_t>(n * 0.75)]
  };
}
