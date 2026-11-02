#pragma once

#include <chrono>
#include <cstdint>

namespace NXKit::Internal {

// Opt-in, main-thread instrumentation for regression/performance harnesses.
// Timings are inclusive CPU wall time; shader execution is synchronous only
// on a raster canvas. A GPU submission timer is not GPU execution time.
struct RenderPerformance {
    inline static thread_local RenderPerformance* current = nullptr;
    std::uint64_t views = 0, layers = 0, rejected = 0, sorts = 0, glass = 0;
    double layoutMs = 0, renderMs = 0, glassMs = 0, sortMs = 0, submitMs = 0;
};

class RenderPerformanceScope {
public:
    explicit RenderPerformanceScope(double* destination): _destination(destination) {
        if (_destination) _start = Clock::now();
    }
    ~RenderPerformanceScope() {
        if (_destination) *_destination += std::chrono::duration<double, std::milli>(Clock::now() - _start).count();
    }
private:
    using Clock = std::chrono::steady_clock;
    double* _destination;
    Clock::time_point _start;
};

}
