#pragma once

#include <UITraitCollection.h>
#include <algorithm>
#include <cmath>

namespace NXKit::Internal {

// Apple's size/contrast thresholds are private. This portable policy matches
// the controlled 56/64-point and 160-point UIKit fixtures; the intermediate
// size boundary is an approximation, not a recovered UIKit constant.
inline bool isSmallGlass(NXSize size) {
    return std::isfinite(size.width) && std::isfinite(size.height)
        && size.width > 0 && size.height > 0
        && std::min(size.width, size.height) < 100.0f;
}

inline UIUserInterfaceStyle glassAppearance(
    NXSize size, UIUserInterfaceStyle inherited, NXFloat luminance,
    UIUserInterfaceStyle previous
) {
    if (!isSmallGlass(size) || !std::isfinite(luminance)) return inherited;
    // Mid-contrast content keeps the app's appearance. Only sufficiently dark
    // or bright content overrides it; a dead band prevents flickering while
    // dragging across a contrast boundary.
    const auto threshold = inherited == UIUserInterfaceStyle::dark ? 0.75f : 0.25f;
    const auto hysteresis = previous == UIUserInterfaceStyle::dark ? 0.035f : -0.035f;
    return luminance < threshold + hysteresis
        ? UIUserInterfaceStyle::dark : UIUserInterfaceStyle::light;
}

}
