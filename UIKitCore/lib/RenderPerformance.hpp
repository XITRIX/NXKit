#pragma once

#include <chrono>
#include <cstdint>

namespace NXKit::Internal {

// Opt-in, main-thread instrumentation for regression/performance harnesses.
// Timings are inclusive CPU wall time; shader execution is synchronous only
// on a raster canvas. A GPU submission timer is not GPU execution time.
struct RenderPerformance {
    inline static thread_local RenderPerformance* current = nullptr;
    // Private opt-in native frame diagnostics; absent in normal application runs.
    inline static void (*observer)(const RenderPerformance&) = nullptr;
    inline static const char* phase = "manual";
    inline static std::uint64_t sequence = 0;
    std::uint64_t frame = 0;
    bool presented = false;
    std::uint64_t views = 0, layers = 0, rejected = 0, sorts = 0, glass = 0;
    std::uint64_t boundedMasks = 0, isolatedMasks = 0, preservedMaskPixels = 0, boundedLenses = 0, boundedLensGroups = 0, croppedLensSources = 0, lensSourcePixels = 0;
    std::uint64_t metalLibraryCompiles = 0, metalPipelineCompiles = 0, metalArchiveHits = 0;
    double layoutMs = 0, renderMs = 0, glassMs = 0, sortMs = 0, submitMs = 0;
    double acquireMs = 0, totalMs = 0;
    std::uint64_t uniformMaskPrefixes = 0;
    std::uint64_t metalAllocatedBytes = 0, graphiteBudgetedBytes = 0, graphitePurgeableBytes = 0;
    double memorySampleTime = 0;
};

class RenderFrameMeasurement {
public:
    RenderFrameMeasurement(): previous(RenderPerformance::current) {
        if (RenderPerformance::observer && !previous) {
            start = std::chrono::steady_clock::now();
            value.frame = ++RenderPerformance::sequence;
            RenderPerformance::current = &value;
            measuring = true;
        }
    }
    ~RenderFrameMeasurement() {
        if (!measuring) return;
        value.totalMs = std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
        RenderPerformance::current = previous;
        if (RenderPerformance::observer) RenderPerformance::observer(value);
    }
private:
    RenderPerformance value;
    RenderPerformance* previous;
    std::chrono::steady_clock::time_point start;
    bool measuring = false;
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
