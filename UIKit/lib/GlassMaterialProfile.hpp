#pragma once
#include <UIGlassEffect.h>
#include "NativeGlassProfile.hpp"

namespace NXKit::Internal {
struct GlassMaterialParameters {
    NXFloat maximumSampleRadius;
    NXFloat frostRadius;
    NXFloat refraction;
    NXFloat refractionDepth;
    NXFloat dispersion;
    NXFloat saturation;
    NXFloat contrast;
    NXFloat luminosity;
    UIColor tintColor;
    NXFloat tintOpacity;
    NXFloat edgeStrength;
    NXFloat specularStrength;
    NXFloat darkBackdropSpecularStrength;
    NXFloat brightBackdropSpecularStrength;
    NXFloat lightAngle;
    NXFloat specularDirectionalPower;
    NXFloat specularWidth;
    NXFloat reflectionSampleDistance;
    NXFloat reflectionSpread;
    NXFloat brightBackdropShade;
};
// Framebuffer-calibrated conversion from native CAFilter input space to the
// portable optical model. Explicit UIGlassEffect overrides are applied later.
GlassMaterialParameters translateGlassProfile(const NativeGlassProfile& profile,
    UIGlassEffect::Style style, NXSize size, UIUserInterfaceStyle interfaceStyle,
    bool hasNativeTint);
GlassMaterialParameters resolveGlassMaterial(UIGlassEffect::Style style, NXSize size,
    UIUserInterfaceStyle interfaceStyle, NXFloat glassTintAmount, bool hasNativeTint);
}
