#pragma once
#include <Geometry.h>
#include "include/core/SkRefCnt.h"
class SkImageFilter;class SkShader;
namespace NXKit { class UIBlurEffect; }
namespace NXKit::Internal {
// A one-dimensional mask permits exact separable ordering: filter along the
// gradient first, then across it. Each output samples its own Gaussian sigma.
sk_sp<SkImageFilter> progressiveEdgeBlurFilter(NXFloat radius,sk_sp<SkShader> mask,NXSize viewport,NXFloat scale,bool verticalGradient);
sk_sp<SkImageFilter> progressiveBlurFilter(NXFloat radius,sk_sp<SkShader> mask,NXFloat scale=1);
sk_sp<SkImageFilter> progressiveBlurFilter(const UIBlurEffect&,NXSize size,NXFloat scale);
}
