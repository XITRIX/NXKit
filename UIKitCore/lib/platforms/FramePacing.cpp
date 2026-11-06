#include <SkiaCtx.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <string_view>
#include <thread>

using namespace NXKit;

namespace {
bool uncappedRendering() {
    // Profiling override shared by SDL event waits and the fallback sleeper.
    // Read once at launch; drawable acquisition can still apply display pacing.
    static const bool uncapped = [] {
        const auto value = std::getenv("NXKIT_UNCAPPED");
        return value && std::string_view(value) == "1";
    }();
    return uncapped;
}
}

double SkiaCtx::remainingFrameMilliseconds(FrameClock::time_point now) {
    if (uncappedRendering() || !_hasScheduledFrame) return 0;
    const auto rate = float(targetFrameRate > 0 ? targetFrameRate : screenFrameRate());
    const auto elapsed = std::chrono::duration<double,std::milli>(now-_lastScheduledFrame).count();
    return std::max(0.0, 1000.0 / std::max(1.0f, rate) - elapsed);
}

bool SkiaCtx::beginScheduledFrame(FrameClock::time_point now) {
    if (remainingFrameMilliseconds(now) > .001) return false;
    if (_hasScheduledFrame && !uncappedRendering()) {
        const auto rate = double(targetFrameRate > 0 ? targetFrameRate : screenFrameRate());
        const auto period = std::chrono::duration<double>(1.0 / std::max(1.0, rate));
        const auto elapsed = std::chrono::duration<double>(now - _lastScheduledFrame);
        // Keep the display phase across small scheduler overruns. Resetting to
        // now on every wake accumulates sleep jitter and lowers the frame rate.
        // Skip missed intervals after a stall; never enqueue catch-up frames.
        const auto intervals = std::max(1.0, std::floor(elapsed / period));
        _lastScheduledFrame += std::chrono::duration_cast<FrameClock::duration>(period * intervals);
    } else _lastScheduledFrame = now;
    _hasScheduledFrame = true;
    return true;
}

int SkiaCtx::millisecondsUntilNextFrame() {
    return int(remainingFrameMilliseconds());
}

void SkiaCtx::sleepForNextFrame() {
    const auto milliseconds = remainingFrameMilliseconds();
    if (milliseconds > 0) std::this_thread::sleep_for(std::chrono::duration<double,std::milli>(milliseconds));
}
