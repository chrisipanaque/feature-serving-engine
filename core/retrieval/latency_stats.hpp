#ifndef CORE_RETRIEVAL_LATENCY_STATS_HPP
#define CORE_RETRIEVAL_LATENCY_STATS_HPP

#include <cstdint>

struct LatencyStats {
    double avg_us;
    double p50_us;
    double p99_us;
};

#endif
