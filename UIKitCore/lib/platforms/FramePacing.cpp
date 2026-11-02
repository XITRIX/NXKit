#include <SkiaCtx.h>
#include <algorithm>
#include <chrono>
#include <thread>

using namespace NXKit;

int SkiaCtx::millisecondsUntilNextFrame() {
    const auto rate = float(targetFrameRate > 0 ? targetFrameRate : screenFrameRate());
    return std::max(0, int(1000.0f / std::max(1.0f, rate) - currentRunLoopStartTimer.getElapsedTimeInMilliseconds()));
}

void SkiaCtx::sleepForNextFrame() {
    const auto milliseconds = millisecondsUntilNextFrame();
    if (milliseconds > 0) std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
