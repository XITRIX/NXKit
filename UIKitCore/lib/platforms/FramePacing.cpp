#include <SkiaCtx.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <string_view>
#include <thread>

using namespace NXKit;

int SkiaCtx::millisecondsUntilNextFrame() {
    // Profiling override shared by SDL event waits and the fallback sleeper.
    // Read once at launch; drawable acquisition can still apply display pacing.
    static const bool uncapped = [] {
        const auto value = std::getenv("NXKIT_UNCAPPED");
        return value && std::string_view(value) == "1";
    }();
    if (uncapped) return 0;
    const auto rate = float(targetFrameRate > 0 ? targetFrameRate : screenFrameRate());
    return std::max(0, int(1000.0f / std::max(1.0f, rate) - currentRunLoopStartTimer.getElapsedTimeInMilliseconds()));
}

void SkiaCtx::sleepForNextFrame() {
    const auto milliseconds = millisecondsUntilNextFrame();
    if (milliseconds > 0) std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
