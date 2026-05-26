#ifndef CORE_PROFILING_TIMER_HPP
#define CORE_PROFILING_TIMER_HPP

#include <chrono>

class ScopedTimer {
public:
    explicit ScopedTimer(double& out_us)
        : out_(out_us)
        , start_(std::chrono::high_resolution_clock::now())
    {}

    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        out_ = std::chrono::duration<double, std::micro>(end - start_).count();
    }

private:
    double& out_;
    std::chrono::high_resolution_clock::time_point start_;
};

#endif
