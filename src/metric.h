#ifndef METRIC_H
#define METRIC_H

#include <cstdint>
#include <ctime>
#include <vector>

class Metric;

/**
 * RAII pattern to record seconds of CPU time into a metric.
 */
class TimeMetricRAII {
  clock_t start;
  Metric& metric;
public:
  TimeMetricRAII(Metric& metric);
  ~TimeMetricRAII();
};

struct MetricReport {
  const float max;
  const float p99;
  const float p75;
};

/**
 * Record a sliding window of values and provide percentile stats.
 */
class Metric {
  uint32_t i = 0;
  std::vector<float> values;
public:
  Metric(uint32_t length);
  void record(float value);

  MetricReport report() const;
};

#endif
