#include <algorithm>
#include "./metric.h"

using namespace std;

static float getSecondsElapsed(const timespec& a, const timespec &b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

TimeMetricRAII::TimeMetricRAII(Metric& metric): metric(metric) {
  clock_gettime(CLOCK_MONOTONIC, &start);
}

TimeMetricRAII::~TimeMetricRAII() {
  timespec cur;
  clock_gettime(CLOCK_MONOTONIC, &cur);
  metric.record(getSecondsElapsed(start, cur));
}

std::function<float()> stopWatchSeconds() {
  timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);

  return [start]() {
    timespec cur;
    clock_gettime(CLOCK_MONOTONIC, &cur);
    return getSecondsElapsed(start, cur);
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
