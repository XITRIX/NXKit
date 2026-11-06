#pragma once

#include <Geometry.h>
#include <string_view>

namespace NXKit {
class BackdropEffect;
namespace Internal {
struct NativeGlassProfile;

// The recovered values are native inputs. Equations independently translated
// from the shipped QuartzCore AIR are documented alongside native framebuffer
// controls; unsupported host packing and renderer differences remain explicit.
std::string_view nativeGlassLightingUniforms();
std::string_view nativeGlassLightingShaderSource();
// maxHeadroom in the profile is a cap, not the display's available headroom.
// Existing render targets are SDR; a future HDR target passes its actual value.
void bindNativeGlassLighting(const NativeGlassProfile&, BackdropEffect&,
                            NXFloat displayHeadroom = 1);
NXFloat nativeGlassExteriorOutset(const NativeGlassProfile&);
NXPoint nativeGlassShadowOffset(const NativeGlassProfile&);
NXFloat nativeGlassRingOffset(const NativeGlassProfile&);
}
}
