#pragma once

#include "NativeGlassProfile.hpp"

namespace NXKit::Internal {

// Native filter units, before source-texture coordinate conversion. The SDF
// normal matrix is part of that conversion; amount is not a unit-normal gain.
struct NativeLensDisplacement {
    NXFloat amount, height, curvature, angle, maskOffset, effectOffset;
    NXFloat gradientOvalization;
    bool hasGradientOvalization;
    std::string_view sourceSublayerName;
};

struct NativeLensForeground {
    NXFloat refractionAmount, refractionHeight, refractionOffset;
    NXFloat aberrationAmount, aberrationHeight, aberrationOffset, aberrationAngle;
    NXFloat edgeStart, edgeEnd, edgeOpacityStart, edgeOpacityEnd;
    std::string_view sourceSublayerName;
    NXFloat captureMargin;
    NativeGlassOutput output;
};

struct NativeLensSpring {
    NXFloat response, dampingRatio, retargetImpulse;
    NXFloat trackingResponse, trackingDampingRatio, trackingRetargetImpulse;
    NXFloat responseSmoothing, dampingRatioSmoothing;
    NXFloat trackingResponseSmoothing, trackingDampingRatioSmoothing;
    NXFloat inertialTargetSmoothingRatio, inertialProjectionDeceleration;
    NXFloat deceleration;
    int behaviorType;
    bool smoothingAndProjectionEnabled;
};

struct NativeLensSpec {
    NXFloat liftedDisplacement, unliftedDisplacement, contentWrapperDisplacement;
    NXFloat unliftedBlurRadius, lensHangTime;
    NXFloat innerShadowOffsetY, innerShadowOpacity, innerShadowRadius;
    NativeLensSpring liftSpring, unliftSpring;
    std::string_view className;
};

struct NativeLensProfile {
    NativeLensDisplacement backdrop, contentWrapper, source;
    NXFloat backdropBlurRadius;
    NativeLensForeground foreground;
    NativeGlassProfile glass;
    // The lens archive carries this additional, currently transparent input.
    // NativeGlassProfile's regular-glass archive schema does not include it.
    NativeGlassColor bleedColorMatrixFillColor;
    // Missing input values remain distinguishable from captured values. The
    // corresponding glass fields contain neutral portable fallback values.
    bool hasGlassClamp, hasGlassClampPreserveHue;
    NativeLensSpec spec;
};

// iOS 27.0 / 24A434, explicitly selected small/large variants. The six captured
// sizes establish shortest-side rules; their extension to other finite sizes
// is a portable policy. Empty size resolves all size-dependent distances to0.
// Negative/nonfinite geometry rejects. Preferences 0/.5/1 are checked by the
// generator; this resolver does not assert unmeasured preference/HDR behavior.
NativeLensProfile nativeLensProfile(bool selection, bool dark, NXSize size);

} // namespace NXKit::Internal
