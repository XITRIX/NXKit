#include "SkiaCtx.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include <thread>

using namespace NXKit;

std::shared_ptr<SkiaCtx> SkiaCtx::_main;

void SkiaCtx::flushAndSubmit(sk_sp<SkSurface> surface) {
    if (auto dContext = directContext()) {
        dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    }
}

void SkiaCtx::sleepForNextFrame() {
    auto _screenFrameRate = float(targetFrameRate > 0 ? targetFrameRate : screenFrameRate());
    auto frameTime = int(1000.f / _screenFrameRate - currentRunLoopStartTimer.getElapsedTimeInMilliseconds());
    printf("Frame time: %d\n", frameTime);
    if (frameTime > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(frameTime));
    }
}