#pragma once
#include <BackdropEffect.h>
#include "NativeGlassProfile.hpp"

namespace NXKit::Internal {
// Private composition hook for other UIKit materials using the same native
// glass-background pass. The portable Core has no component-specific policy.
std::string_view nativeGlassMaterialShaderSource();
// Independent translation of the native pass equations. Distances use local
// points; the portable renderer supplies the source pyramid and geometry field.
BackdropEffect makeNativeGlassEffect(const NativeGlassProfile&, UIGlassEffect::Style,
    UIColor tint, bool hasAccent, NXFloat maximumSampleRadius = 128,
    std::string_view shaderSourceOverride = {});
}
