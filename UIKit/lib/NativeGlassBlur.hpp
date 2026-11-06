#pragma once
#include <BackdropEffect.h>
#include <string_view>

namespace NXKit::Internal {
// Native AIR stencil and radius-to-LOD law. Capture resampling remains independent;
// the portable atlas reconstructs explicit trilinear sampling with one child.
std::string_view nativeGlassBlurShaderSource();
// Default is the complete native fragment path. The compute variant is kept
// distinct: its integer offsets are not interchangeable with fragment taps.
const std::vector<std::array<NXFloat,3>>& nativeGlassPyramidTaps(bool compute=false);
NXFloat nativeGlassBlurLOD(NXFloat radius,NXFloat displayScale,NXFloat captureScale,NXFloat minimumRadius=0);
}
