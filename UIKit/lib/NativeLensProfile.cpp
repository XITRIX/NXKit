#include "NativeLensProfile.hpp"
#include "NativeLensProfileData.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace NXKit::Internal {

NativeLensProfile nativeLensProfile(bool selection, bool dark, NXSize size) {
    if (!std::isfinite(size.width) || !std::isfinite(size.height)
        || size.width < 0 || size.height < 0)
        throw std::invalid_argument("Native lens size must be finite and nonnegative");
    auto result = nativeLensProfiles[(selection ? 2 : 0) + (dark ? 1 : 0)];
    const auto shortSide = std::min(size.width, size.height);
    result.source.amount = selection ? -.2f * shortSide : 0;
    result.source.height = selection ? .16f * shortSide : 0;
    result.foreground.aberrationAmount = selection ? shortSide / 19.f : 0;
    result.foreground.aberrationOffset = shortSide / 1.8f;
    result.foreground.edgeStart = -.2f * shortSide;
    result.glass.innerRefractionAmount = -(selection ? .15f : .3f) * shortSide;
    result.glass.innerRefractionHeight = std::min(20.f, (selection ? .1f : .18f) * shortSide);
    result.glass.shadowAmount = .25f * shortSide;
    // Only80 and120pt bracket the observed spread change. Interpolation and
    // clamping between/outside these anchors are portable policies.
    const auto spreadProgress = std::clamp((shortSide - 80.f) / 40.f, 0.f, 1.f);
    const auto spread = nativeLensProfiles[0].glass.highlight.keySpread
        + (nativeLensWideHighlightSpread - nativeLensProfiles[0].glass.highlight.keySpread) * spreadProgress;
    result.glass.highlight.keySpread = spread;
    result.glass.highlight.fillSpread = spread;
    // Retain actual captured provenance, including its reference geometry.
    // The resolved geometry is the caller's size, not a new native capture.
    return result;
}

} // namespace NXKit::Internal
