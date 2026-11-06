#pragma once
#include <UIGlassEffect.h>
#include <array>
#include <string_view>

namespace NXKit::Internal {
// Captured values retain native filter units. Translation into Skia sampling,
// color spaces and SDF operations belongs to the portable renderer.
struct NativeGlassColor { NXFloat r, g, b, a; };
struct NativeGlassOffset { NXFloat x, y; };
struct NativeGlassMatrix { std::array<NXFloat, 20> values; }; // Row-major 4x5 RGBA+bias.
struct NativeGlassVibrantColorMatrix {
    NativeGlassMatrix matrix;
    NXFloat clamp;
    bool clampPreserveHue;
    bool backdropAware;
    bool hasBackdropAware; // Absent input is preserved, not asserted as a native default.
};
struct NativeGlassOutput { NXFloat minimum, maximum; };
struct NativeGlassHighlight {
    NXFloat curvature;
    NXFloat keyAngle, keyHeight, keyHeightScale, keyHeightOffset;
    NXFloat keyAmount, keySpread, keySpreadScale, keySpreadOffset;
    NXFloat fillAngle, fillHeight, fillHeightScale, fillHeightOffset;
    NXFloat fillAmount, fillSpread, fillSpreadScale, fillSpreadOffset;
    NXFloat diffuseAmountScale, diffuseSpreadScale, diffuseHeightScale;
    NativeGlassColor keyColor, fillColor;
    bool global;
    NativeGlassVibrantColorMatrix vibrant;
};
struct NativeGlassAccent {
    std::array<NativeGlassColor, 3> colors;
    std::array<NXFloat, 3> distances;
    bool premultiplied;
    std::array<std::string_view, 2> interpolationClasses;
    // New captures read public CAMediaTimingFunction controls directly. Older
    // archives expose only classes: unavailable controls remain explicitly false.
    std::array<std::array<NXFloat, 4>, 2> interpolationControlPoints;
    std::array<bool, 2> hasInterpolationControlPoints;
    NativeGlassColor fillColor;
    std::string_view fillCompositingFilter;
    NativeGlassVibrantColorMatrix vibrant;
};
struct NativeGlassContext {
    std::string_view runtime, build;
    NXFloat width, height, cornerRadius, displayScale, backdropScale;
    std::string_view background, capturedTintColor;
};
struct NativeGlassProfile {
    NXFloat bleedAmount; // inputBleedAmount
    NXFloat bleedBlurRadius; // inputBleedBlurRadius
    NXFloat bleedColorMatrixBlack; // inputBleedColorMatrixBlack
    NXFloat bleedColorMatrixSaturation; // inputBleedColorMatrixSaturation
    NXFloat bleedColorMatrixWhite; // inputBleedColorMatrixWhite
    bool bleedDarkenBlend; // inputBleedDarkenBlend
    NXFloat bleedDistance0; // inputBleedDistance0
    NXFloat bleedDistance1; // inputBleedDistance1
    NXFloat bleedHeight; // inputBleedHeight
    NXFloat bleedOpacity; // inputBleedOpacity
    NXFloat blurDistance0; // inputBlurDistance0
    NXFloat blurDistance1; // inputBlurDistance1
    NXFloat blurDistance2; // inputBlurDistance2
    NXFloat blurDistance3; // inputBlurDistance3
    NXFloat blurFillRadius; // inputBlurFillBlurRadius
    NXFloat blurFillDarkenOpacity; // inputBlurFillDarkenOpacity
    NXFloat blurFillLightenOpacity; // inputBlurFillLightenOpacity
    NXFloat blurFillNormalOpacity; // inputBlurFillNormalOpacity
    NXFloat blurOpacity0; // inputBlurOpacity0
    NXFloat blurOpacity1; // inputBlurOpacity1
    NXFloat blurOpacity2; // inputBlurOpacity2
    NXFloat blurOpacity3; // inputBlurOpacity3
    NXFloat blurRadius; // inputBlurRadius
    NXFloat clamp; // inputClamp
    bool clampPreserveHue; // inputClampPreserveHue
    NXFloat faceBlack; // inputFaceColorMatrixBlack
    NativeGlassColor faceColorMatrixFillColor; // inputFaceColorMatrixFillColor
    NXFloat faceColorMatrixMaxLuma; // inputFaceColorMatrixMaxLuma
    NXFloat faceColorMatrixMaxLumaSDR; // inputFaceColorMatrixMaxLumaSDR
    NXFloat faceSaturation; // inputFaceColorMatrixSaturation
    NXFloat faceWhite; // inputFaceColorMatrixWhite
    NXFloat faceOpacity; // inputFaceOpacity
    NXFloat innerRefractionAmount; // inputInnerRefractionAmount
    NXFloat innerRefractionHeight; // inputInnerRefractionHeight
    NXFloat keyFillHighlightAmount; // inputKeyFillHighlightAmount
    NXFloat keyFillHighlightAngle; // inputKeyFillHighlightAngle
    NXFloat keyFillHighlightColorBias; // inputKeyFillHighlightColorBias
    NXFloat keyFillHighlightEffectOffset; // inputKeyFillHighlightEffectOffset
    NXFloat keyFillHighlightHeight; // inputKeyFillHighlightHeight
    NXFloat keyFillHighlightSpread; // inputKeyFillHighlightSpread
    NXFloat keyFillHighlightSpreadSDR; // inputKeyFillHighlightSpreadSDR
    NXFloat maxHeadroom; // inputMaxHeadroom
    NXFloat outerRefractionAmount; // inputOuterRefractionAmount
    NXFloat outerRefractionHeight; // inputOuterRefractionHeight
    NXFloat refractionDistance0; // inputRefractionDistance0
    NXFloat refractionDistance1; // inputRefractionDistance1
    NXFloat refractionOpacity; // inputRefractionOpacity
    NXFloat ringShadowBlurRadius; // inputRingShadowBlurRadius
    NXFloat ringShadowMask; // inputRingShadowMask
    NXFloat ringShadowOffset; // inputRingShadowOffset
    NXFloat ringShadowOpacity; // inputRingShadowOpacity
    NXFloat ringShadowStrokeWidth; // inputRingShadowStrokeWidth
    NXFloat sdrGradientDistance0; // inputSDRGradientDistance0
    NXFloat sdrGradientDistance1; // inputSDRGradientDistance1
    bool sdrHoldingToneEnabled; // inputSDRHoldingToneEnabled
    NXFloat sdrHoldingToneWhite; // inputSDRHoldingToneWhite
    NXFloat sdrShadowOpacity; // inputSDRShadowOpacity
    NXFloat shadowAmount; // inputShadowAmount
    NXFloat shadowBlurRadius; // inputShadowBlurRadius
    NXFloat shadowColorMatrixBlack; // inputShadowColorMatrixBlack
    NativeGlassColor shadowColorMatrixFillColor; // inputShadowColorMatrixFillColor
    NXFloat shadowColorMatrixSaturation; // inputShadowColorMatrixSaturation
    NXFloat shadowColorMatrixWhite; // inputShadowColorMatrixWhite
    NXFloat shadowDistanceOffset; // inputShadowDistanceOffset
    NXFloat shadowHeight; // inputShadowHeight
    NativeGlassOffset shadowOffset; // inputShadowOffset
    NXFloat shadowOpacity; // inputShadowOpacity
    NXFloat shadowRadius; // inputShadowRadius
    NXFloat shadowVibrancyContribution; // inputShadowVibrancyContribution
    std::string_view sourceSublayerName; // inputSourceSublayerName
    NativeGlassOutput output;
    NativeGlassHighlight highlight;
    // Regular-with-systemBlue uses the same backdrop dictionary as regular;
    // this independent accent pass is applied only when an explicit tint exists.
    NativeGlassAccent accent;
    NativeGlassContext context;
    std::string_view captureName, backdropArchiveSHA256, layerSourceSHA256;
};

// iOS 27.0 / 24A434, 280x80pt, radius 40pt, scale 3. Piecewise linear
// interpolation across measured 0/.5/1 samples is an NXKit policy, not a
// recovered UIKit law. Face-fill RGBA filter coefficients interpolate straight;
// gradient stops honor their native premultiplied flag. Shadow/highlight colors
// interpolate premultiplied. Flags and metadata select the nearest endpoint
// (ties select the upper endpoint). No geometry
// scaling or extrapolation, and no claim of generic HDR/accessibility coverage.
NativeGlassProfile interpolateNativeGlassProfiles(const NativeGlassProfile& a,
    const NativeGlassProfile& b, NXFloat progress);
NativeGlassProfile nativeGlassProfile(UIGlassEffect::Style style,
    UIUserInterfaceStyle interfaceStyle, NXFloat tintAmount);
// Size-resolved iOS 27 captures: shortest dimensions 44/52/62/64/80/120/160,
// measured tint preferences 0/.25/.5/.75/1, circular and 280pt-wide contexts.
// All applied settings match across those two aspect families at each size. Interpolation between measured
// shortest dimensions and clamping outside the captured range are NXKit
// policies. Context geometry/source identities retain the nearest real
// capture; they never claim an interpolated sample was directly observed.
// Empty size preserves the original 280x80 fixture; negative/nonfinite rejects.
NativeGlassProfile nativeGlassProfile(UIGlassEffect::Style style,
    UIUserInterfaceStyle interfaceStyle, NXFloat tintAmount, NXSize size);
// Explicitly tinted materials have their own captured backdrop/highlight stack.
NativeGlassProfile nativeGlassProfile(UIGlassEffect::Style style,
    UIUserInterfaceStyle interfaceStyle, NXFloat tintAmount, NXSize size, bool tinted);
// Resolve the native pixel-sized microcontour from the view's display-scale
// trait before caller overrides. Captured source context remains provenance.
// Native controls cover1x/2x/2.5x/3x. Selecting the higher-density recipe above
// 2x for other fractional scales is a portable policy, not a recovered threshold.
void resolveNativeGlassDisplayScale(NativeGlassProfile&, NXFloat displayScale);
}
