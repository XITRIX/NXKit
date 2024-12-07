#include "SkiaCtx.h"
#include "include/gpu/ganesh/GrDirectContext.h"

using namespace NXKit;

void SkiaCtx::flushAndSubmit(sk_sp<SkSurface> surface) {
    if (auto dContext = directContext()) {
        dContext->flushAndSubmit(surface.get(), GrSyncCpu::kYes);
    }
}
