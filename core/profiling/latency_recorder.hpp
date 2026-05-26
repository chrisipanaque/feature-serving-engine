#ifndef CORE_PROFILING_LATENCY_RECORDER_HPP
#define CORE_PROFILING_LATENCY_RECORDER_HPP

#include <algorithm>
#include <mutex>
#include <numeric>
#include <vector>

struct LatencySummary {
    double avg_us;
    double p50_us;
    double p99_us;
};

class LatencyRecorder {
public:
    void record(double us) {
        std::lock_guard<std::mutex> lock(mtx_);
        samples_.push_back(us);
    }

    LatencySummary summarize() const {
        std::lock_guard<std::mutex> lock(mtx_);
        if (samples_.empty())
            return {0, 0, 0};

        auto sorted = samples_;
        std::sort(sorted.begin(), sorted.end());

        double sum = std::accumulate(sorted.begin(), sorted.end(), 0.0);
        size_t n = sorted.size();

        return {
            sum / static_cast<double>(n),
            sorted[n / 2],
            sorted[static_cast<size_t>(n * 0.99)]
        };
    }

    size_t count() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return samples_.size();
    }

private:
    mutable std::mutex mtx_;
    std::vector<double> samples_;
};

#endif
