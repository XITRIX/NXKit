#include <SkiaCtx.h>
#include "include/core/SkData.h"
#include <stdexcept>
using namespace NXKit;
std::shared_ptr<SkiaCtx> SkiaCtx::_main;
void SkiaCtx::readBackdrop(sk_sp<SkImage>, BackdropCompletion completion) { completion(std::nullopt); }

// Headless tests have no GPU submission or frame pacing.
void SkiaCtx::flushAndSubmit(sk_sp<SkSurface>) {}

void SkiaCtx::pollBackdropReads() {}
std::unique_ptr<SkiaCtx> NXKit::MakeSkiaCtx() {
    throw std::runtime_error("Headless tests supply their own SkiaCtx");
}
