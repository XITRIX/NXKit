#include "GlassMaterialProfile.hpp"
#include <algorithm>
#include <cmath>

using namespace NXKit;
using namespace NXKit::Internal;

namespace {

NXFloat smoothstep(NXFloat lower, NXFloat upper, NXFloat value) {
    const auto progress = std::clamp((value - lower) / (upper - lower), 0.0f, 1.0f);
    return progress * progress * (3.0f - 2.0f * progress);
}

NXFloat interpolate(NXFloat lower, NXFloat upper, NXFloat progress) {
    return lower + (upper - lower) * progress;
}

GlassMaterialParameters styleDefaults(
    UIGlassEffect::Style style,
    NXSize size,
    UIUserInterfaceStyle interfaceStyle,
    bool hasNativeTint
) {
    if (style == UIGlassEffect::Style::clear) {
        return {
            .maximumSampleRadius = 64.0f,
            .frostRadius = 2.0f,
            .refraction = 0.55f,
            .refractionDepth = 0.90f,
            .dispersion = 0.01f,
            .saturation = 1.0f,
            .contrast = 1.04f,
            .luminosity = 0.12f,
            .tintColor = UIColor::white,
            .tintOpacity = 0.0f,
            .edgeStrength = 1.0f,
            .specularStrength = 0.52f,
            .darkBackdropSpecularStrength = 0.52f,
            .brightBackdropSpecularStrength = 0.14f,
            .lightAngle = interfaceStyle == UIUserInterfaceStyle::dark ? 90.0f : -90.0f,
            .specularDirectionalPower = 3.0f,
            .specularWidth = 0.8f,
            .reflectionSampleDistance = 12.0f,
            .reflectionSpread = 3.0f,
            .brightBackdropShade = 0.0f,
        };
    }

    // Matched gallery captures hold app appearance fixed while changing the
    // backdrop. Regular glass retains a neutral body even on solid black.
    const auto shortestDimension = std::max<NXFloat>(0, std::min(size.width, size.height));
    const auto sizeAmount = smoothstep(64.0f, 160.0f, shortestDimension);
    const bool dark = interfaceStyle == UIUserInterfaceStyle::dark;
    const auto bodySize = std::clamp((shortestDimension - 52.0f) / 108.0f, 0.0f, 1.0f);
    const auto darkSize = std::clamp((shortestDimension - 52.0f) / 68.0f, 0.0f, 1.0f);
    return {
        // Keep the declared backdrop extent at the proven cross-platform
        // limit used by the existing glass renderer. The largest regular
        // frost kernel still fits comfortably inside it.
        .maximumSampleRadius = 64.0f,
        .frostRadius = hasNativeTint ? interpolate(3.0f, 6.0f, sizeAmount)
            : (dark ? 12.0f : interpolate(6.0f, 12.0f, sizeAmount)),
        .refraction = interpolate(0.35f, 0.55f, sizeAmount),
        .refractionDepth = interpolate(0.70f, 0.90f, sizeAmount),
        .dispersion = 0.01f,
        .saturation = dark ? interpolate(1.33764f, 1.34934f, darkSize) : interpolate(1.92f, 1.965f, bodySize),
        .contrast = dark ? interpolate(1.05155f, 0.95258f, darkSize) : interpolate(1.015f, 1.113f, bodySize),
        .luminosity = 0.0f,
        .tintColor = interfaceStyle == UIUserInterfaceStyle::dark ? UIColor::white : UIColor::systemBackground,
        .tintOpacity = dark ? interpolate(0.1261f, 0.12395f, darkSize) : interpolate(0.5201f, 0.53806f, bodySize),
        .edgeStrength = 1.0f,
        .specularStrength = 0.52f,
        .darkBackdropSpecularStrength = 0.52f,
        .brightBackdropSpecularStrength = 0.52f,
        .lightAngle = interfaceStyle == UIUserInterfaceStyle::dark ? 90.0f : -90.0f,
        .specularDirectionalPower = 3.0f,
        .specularWidth = 0.8f,
        .reflectionSampleDistance = 12.0f,
        .reflectionSpread = 3.0f,
        .brightBackdropShade = interpolate(0.008f, 0.004f, bodySize),
    };
}

}

GlassMaterialParameters NXKit::Internal::translateGlassProfile(
    const NativeGlassProfile& profile, UIGlassEffect::Style style, NXSize size,
    UIUserInterfaceStyle interfaceStyle, bool hasNativeTint
) {
    // These are resolved native pass controls. The mapping below is a measured
    // Skia adapter, not an assertion that CAFilter radii are Gaussian sigma.
    const auto materialPreference = style == UIGlassEffect::Style::regular
        ? profile.blurFillNormalOpacity
        : 0.5f * (profile.blurRadius / 0.75f + profile.blurFillNormalOpacity);
    auto parameters = styleDefaults(style, size, interfaceStyle, hasNativeTint);
    // iOS 27 framebuffer controls: the slider changes regular material density,
    // not view alpha. Clear style retains its body and explicit colors survive
    // at every position. Keep caller-specified optical overrides below intact.
    const bool dark = interfaceStyle == UIUserInterfaceStyle::dark;
    const auto increasedTint = std::max<NXFloat>(0, (materialPreference - 0.5f) * 2);
    // Measured 4-point stripe transmission uses a small, continuous kernel
    // for clear/colored glass. Neutral regular retains its wider context blur
    // plus detail lane; the preference progressively suppresses that lane.
    if (style == UIGlassEffect::Style::clear || hasNativeTint) {
        const auto firstHalf = std::min<NXFloat>(1, materialPreference * 2);
        parameters.frostRadius = interpolate(0.4f, 1.0f, firstHalf);
        if (materialPreference > 0.5f)
            parameters.frostRadius = interpolate(1.0f, 1.65f, increasedTint);
    } else if (materialPreference < 0.5f) {
        parameters.frostRadius = materialPreference <= 0.25f
            ? interpolate(0.7f, 1.1f, materialPreference * 4)
            : interpolate(1.1f, parameters.frostRadius, (materialPreference - 0.25f) * 4);
    }
    if (style == UIGlassEffect::Style::regular && !hasNativeTint) {
        if (dark && increasedTint > 0) {
            const auto first = std::min<NXFloat>(1, increasedTint * 2);
            const auto second = std::max<NXFloat>(0, increasedTint * 2 - 1);
            parameters.contrast *= interpolate(1, 0.735f, first) * interpolate(1, 0.721f, second);
            parameters.saturation *= interpolate(1, 1.025f, first) * interpolate(1, 0.961f, second);
            parameters.tintOpacity = interpolate(parameters.tintOpacity, 0.113f, first);
            parameters.tintOpacity = interpolate(parameters.tintOpacity, 0.129f, second);
        } else if (!dark) {
            parameters.tintOpacity += (materialPreference - 0.5f) * (materialPreference < 0.5f ? 0.24f : 0.36f);
        }
    }

    // Relative transfer keeps the independently measured native framebuffer
    // calibration while making native face and lens settings real inputs.
    // A new capture must be reviewed with its adapter, not silently relabelled.
    const auto nativeSaturation = style == UIGlassEffect::Style::clear ? 1.059999942779541f
        : (dark ? 1.2999999523162842f : 1.2000000476837158f);
    const auto nativeBlack = style == UIGlassEffect::Style::clear || dark ? 0.125f : 0.4000000059604645f;
    const auto nativeWhite = style == UIGlassEffect::Style::clear ? 1.0800000429153442f
        : (dark ? 1.125f : 1.0299999713897705f);
    parameters.saturation *= profile.faceSaturation / nativeSaturation;
    parameters.contrast *= (profile.faceWhite - profile.faceBlack) / (nativeWhite - nativeBlack);
    parameters.luminosity += profile.faceBlack - nativeBlack;
    parameters.refraction *= -profile.innerRefractionAmount
        / (style == UIGlassEffect::Style::clear ? 52.0f : 40.0f);
    parameters.refractionDepth *= profile.innerRefractionHeight / 20.0f;
    if (style == UIGlassEffect::Style::clear) {
        // Main blur and blur fill are distinct controls. This first adapter
        // collapses them to the existing single Gaussian, avoiding another GPU
        // pass until the native blend equations have been measured.
        // A zero native main radius still contains filtering from the other
        // native passes/resampling. Setting Skia sigma to zero failed the
        // independent 4pt-stripe control by 25/255; retain the measured .4pt
        // residual and convert the main radius above it.
        parameters.frostRadius = 0.4f + 0.6f * (profile.blurRadius / 0.75f)
            + 0.65f * (profile.blurFillRadius / 8.0f) * profile.blurFillNormalOpacity;
    } else {
        // Calibrated single-kernel approximation of main + normal fill blur.
        // Keep the two native radii independently editable without adding a
        // second full-resolution backdrop pass to every control.
        const auto normal = std::clamp(profile.blurFillNormalOpacity, 0.0f, 1.0f);
        const auto variance = profile.blurRadius * profile.blurRadius
            + normal * profile.blurFillRadius * profile.blurFillRadius;
        parameters.frostRadius *= std::sqrt(variance / (25.0f + normal * 64.0f));
    }
    return parameters;
}

GlassMaterialParameters NXKit::Internal::resolveGlassMaterial(
    UIGlassEffect::Style style, NXSize size, UIUserInterfaceStyle interfaceStyle,
    NXFloat glassTintAmount, bool hasNativeTint
) {
    return translateGlassProfile(nativeGlassProfile(style, interfaceStyle, glassTintAmount),
        style, size, interfaceStyle, hasNativeTint);
}
