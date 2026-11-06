#include "NativeGlassProfile.hpp"
#include "NativeGlassProfileData.hpp"
#include "NativeGlassSizeProfileData.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace NXKit;
using namespace NXKit::Internal;

namespace {
// Flags are discrete source settings. Choosing the nearest captured endpoint
// avoids manufacturing unsupported fractional modes; ties choose the upper one.
bool interpolateFlag(bool a, bool b, NXFloat progress) {
    return progress < 0.5f ? a : b;
}

NativeGlassColor interpolateColorCoefficients(NativeGlassColor a, NativeGlassColor b, NXFloat progress) {
    return {std::lerp(a.r, b.r, progress), std::lerp(a.g, b.g, progress),
            std::lerp(a.b, b.b, progress), std::lerp(a.a, b.a, progress)};
}

NativeGlassColor interpolateColor(NativeGlassColor a, NativeGlassColor b, NXFloat progress) {
    // Preserve exact measured anchors, including invisible RGB at zero alpha.
    if (progress == 0) return a;
    if (progress == 1) return b;
    const NXFloat alpha = std::lerp(a.a, b.a, progress);
    if (alpha == 0) return {0, 0, 0, 0};
    return {std::lerp(a.r * a.a, b.r * b.a, progress) / alpha,
            std::lerp(a.g * a.a, b.g * b.a, progress) / alpha,
            std::lerp(a.b * a.a, b.b * b.a, progress) / alpha, alpha};
}

NativeGlassVibrantColorMatrix interpolateVibrant(const NativeGlassVibrantColorMatrix& a,
    const NativeGlassVibrantColorMatrix& b, NXFloat progress) {
    auto result = progress < 0.5f ? a : b;
    for (size_t i = 0; i < result.matrix.values.size(); ++i)
        result.matrix.values[i] = std::lerp(a.matrix.values[i], b.matrix.values[i], progress);
    result.clamp = std::lerp(a.clamp, b.clamp, progress);
    result.clampPreserveHue = interpolateFlag(a.clampPreserveHue, b.clampPreserveHue, progress);
    result.backdropAware = interpolateFlag(a.backdropAware, b.backdropAware, progress);
    result.hasBackdropAware = interpolateFlag(a.hasBackdropAware, b.hasBackdropAware, progress);
    return result;
}
}

NativeGlassProfile NXKit::Internal::interpolateNativeGlassProfiles(
    const NativeGlassProfile& a, const NativeGlassProfile& b, NXFloat progress
) {
    if (!std::isfinite(progress) || progress < 0 || progress > 1)
        throw std::invalid_argument("Native glass interpolation progress must be finite and in [0, 1]");
    // Source provenance, textual settings and context select the same endpoint
    // as flags; they are not floating-point optical values.
    auto result = progress < 0.5f ? a : b;
    for (const auto field : nativeGlassScalarFields)
        result.*field = std::lerp(a.*field, b.*field, progress);
    for (const auto field : nativeGlassBoolFields)
        result.*field = interpolateFlag(a.*field, b.*field, progress);
    for (const auto field : nativeGlassColorFields)
        result.*field = field == &NativeGlassProfile::faceColorMatrixFillColor
            ? interpolateColorCoefficients(a.*field, b.*field, progress)
            : interpolateColor(a.*field, b.*field, progress);
    result.shadowOffset = {std::lerp(a.shadowOffset.x, b.shadowOffset.x, progress),
                           std::lerp(a.shadowOffset.y, b.shadowOffset.y, progress)};
    result.output = {std::lerp(a.output.minimum, b.output.minimum, progress),
                     std::lerp(a.output.maximum, b.output.maximum, progress)};
    for (const auto field : nativeGlassHighlightScalarFields)
        result.highlight.*field = std::lerp(a.highlight.*field, b.highlight.*field, progress);
    result.highlight.keyColor = interpolateColor(a.highlight.keyColor, b.highlight.keyColor, progress);
    result.highlight.fillColor = interpolateColor(a.highlight.fillColor, b.highlight.fillColor, progress);
    result.highlight.global = interpolateFlag(a.highlight.global, b.highlight.global, progress);
    result.highlight.vibrant = interpolateVibrant(a.highlight.vibrant, b.highlight.vibrant, progress);
    result.accent.premultiplied = interpolateFlag(a.accent.premultiplied, b.accent.premultiplied, progress);
    for (size_t i = 0; i < result.accent.colors.size(); ++i) {
        result.accent.colors[i] = result.accent.premultiplied
            ? interpolateColor(a.accent.colors[i], b.accent.colors[i], progress)
            : interpolateColorCoefficients(a.accent.colors[i], b.accent.colors[i], progress);
        result.accent.distances[i] = std::lerp(a.accent.distances[i], b.accent.distances[i], progress);
    }
    for (size_t i = 0; i < result.accent.hasInterpolationControlPoints.size(); ++i) {
        result.accent.hasInterpolationControlPoints[i] = interpolateFlag(
            a.accent.hasInterpolationControlPoints[i], b.accent.hasInterpolationControlPoints[i], progress);
        // Unknown archive data is not a numeric zero curve. Interpolate control
        // points only when both endpoints actually measured those controls.
        if (a.accent.hasInterpolationControlPoints[i] && b.accent.hasInterpolationControlPoints[i])
            for (size_t j = 0; j < result.accent.interpolationControlPoints[i].size(); ++j)
                result.accent.interpolationControlPoints[i][j] = std::lerp(
                    a.accent.interpolationControlPoints[i][j], b.accent.interpolationControlPoints[i][j], progress);
    }
    result.accent.fillColor = interpolateColor(a.accent.fillColor, b.accent.fillColor, progress);
    result.accent.vibrant = interpolateVibrant(a.accent.vibrant, b.accent.vibrant, progress);
    return result;
}

NativeGlassProfile NXKit::Internal::nativeGlassProfile(
    UIGlassEffect::Style style, UIUserInterfaceStyle interfaceStyle, NXFloat amount
) {
    if (!std::isfinite(amount) || amount < 0 || amount > 1)
        throw std::invalid_argument("Native glass tint amount must be finite and in [0, 1]");
    const auto base = (interfaceStyle == UIUserInterfaceStyle::dark ? 6 : 0)
        + (style == UIGlassEffect::Style::clear ? 3 : 0);
    const auto interval = amount <= 0.5f ? 0 : 1;
    const auto progress = amount * 2 - interval;
    return interpolateNativeGlassProfiles(nativeGlassProfiles[base + interval],
        nativeGlassProfiles[base + interval + 1], progress);
}


NativeGlassProfile NXKit::Internal::nativeGlassProfile(
    UIGlassEffect::Style style, UIUserInterfaceStyle interfaceStyle, NXFloat amount, NXSize size
) {
    return nativeGlassProfile(style, interfaceStyle, amount, size, false);
}

NativeGlassProfile NXKit::Internal::nativeGlassProfile(
    UIGlassEffect::Style style, UIUserInterfaceStyle interfaceStyle, NXFloat amount, NXSize size, bool tinted
) {
    if (!std::isfinite(amount) || amount < 0 || amount > 1)
        throw std::invalid_argument("Native glass tint amount must be finite and in [0, 1]");
    if (!std::isfinite(size.width) || !std::isfinite(size.height) || size.width < 0 || size.height < 0)
        throw std::invalid_argument("Native glass size must be finite and nonnegative");
    if (size.width == 0 || size.height == 0) return nativeGlassProfile(style, interfaceStyle, amount);
    const auto appearance = interfaceStyle == UIUserInterfaceStyle::dark
        ? UIUserInterfaceStyle::dark : UIUserInterfaceStyle::light;
    const bool circular = size.width == size.height;
    const NXFloat dimension = std::min(size.width, size.height);
    const NativeGlassSizeProfileSet* lower = nullptr;
    const NativeGlassSizeProfileSet* upper = nullptr;
    const auto measuredDimension = [](const NativeGlassSizeProfileSet& set) {
        return std::min(set.profiles[0].context.width, set.profiles[0].context.height);
    };
    for (const auto& set : nativeGlassSizeProfiles) {
        if (set.style != style || set.appearance != appearance || set.circular != circular || set.tinted != tinted) continue;
        const auto candidate = measuredDimension(set);
        if (candidate <= dimension && (!lower || candidate > measuredDimension(*lower))) lower = &set;
        if (candidate >= dimension && (!upper || candidate < measuredDimension(*upper))) upper = &set;
    }
    // Both families cover the same measured sizes. These fallbacks also keep
    // the adapter usable if future captured tables omit a geometry family.
    if (!lower && !upper) return nativeGlassProfile(style, interfaceStyle, amount);
    if (!lower) lower = upper;
    if (!upper) upper = lower;
    const auto tintUpper = std::upper_bound(nativeGlassSizeTintAnchors.begin(), nativeGlassSizeTintAnchors.end(), amount);
    const size_t interval = tintUpper == nativeGlassSizeTintAnchors.end()
        ? nativeGlassSizeTintAnchors.size() - 2
        : size_t(std::max(std::ptrdiff_t(0), tintUpper - nativeGlassSizeTintAnchors.begin() - 1));
    const NXFloat tintProgress = (amount - nativeGlassSizeTintAnchors[interval])
        / (nativeGlassSizeTintAnchors[interval + 1] - nativeGlassSizeTintAnchors[interval]);
    const auto tintSample = [&](const NativeGlassSizeProfileSet& set) {
        return interpolateNativeGlassProfiles(set.profiles[interval], set.profiles[interval + 1], tintProgress);
    };
    auto low = tintSample(*lower);
    if (lower == upper) return low;
    const NXFloat sizeProgress = (dimension - measuredDimension(*lower))
        / (measuredDimension(*upper) - measuredDimension(*lower));
    return interpolateNativeGlassProfiles(low, tintSample(*upper), sizeProgress);
}

void NXKit::Internal::resolveNativeGlassDisplayScale(NativeGlassProfile& profile, NXFloat scale) {
    if (!std::isfinite(scale) || scale <= 0)
        throw std::invalid_argument("Native glass display scale must be finite and positive");
    // Same-screen trait overrides isolate scale from idiom and hardware.
    // All336 independent2x material controls differ from3x only in these fields.
    const bool highDensity = scale > 2;
    profile.keyFillHighlightHeight = (highDensity ? 1.6f : 1.f) / scale;
    profile.keyFillHighlightEffectOffset = -profile.keyFillHighlightHeight;
    profile.keyFillHighlightColorBias = highDensity ? -.3f : -.25f;
}
