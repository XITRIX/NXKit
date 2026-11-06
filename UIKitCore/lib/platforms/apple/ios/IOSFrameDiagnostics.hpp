#pragma once
#include <CoreFoundation/CoreFoundation.h>
#include <cstdint>

namespace NXKit::Internal {
// Retains the real queue when disabled; an instrumented forwarding queue when
// NXKIT_FRAME_METRICS=1. No waits, validation, capture, or rendering changes.
CFTypeRef makeIOSDiagnosticQueue(CFTypeRef queue);
void observeIOSDrawable(CFTypeRef drawable);
void observeIOSPipelineFailures(std::uint64_t failures);
}
