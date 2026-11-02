#pragma once

#include <Geometry.h>
#include "include/core/SkPixmap.h"
#include <optional>

namespace NXKit::Internal {

inline std::optional<NXFloat> backdropLuminance(const SkPixmap& pixels) {
    NXFloat sum = 0, weight = 0;
    for (int y = 0; y < pixels.height(); ++y) for (int x = 0; x < pixels.width(); ++x) {
        const auto color = pixels.getColor(x, y);
        const auto alpha = SkColorGetA(color) / 255.0f;
        sum += alpha * (SkColorGetR(color) * 0.2126f + SkColorGetG(color) * 0.7152f
            + SkColorGetB(color) * 0.0722f) / 255.0f;
        weight += alpha;
    }
    return weight > 0 ? std::optional<NXFloat>(sum / weight) : std::nullopt;
}

}
