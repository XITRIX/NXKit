#pragma once
#include <Geometry.h>
#include <BackdropEffect.h>
#include <algorithm>
#include <string>

namespace NXKit::Internal {

struct GlassShadow {
    NXFloat sigma, offset, opacity;
};

// Native regular-glass exterior profiles, measured at 52–160 points. Clear
// glass does not cast this automatic shadow. Containers share the largest
// member's elevation, including when their members are disconnected.
inline GlassShadow glassShadow(NXSize size, bool dark) {
    const auto elevation = std::clamp((std::min(size.width, size.height) - 64.0f) / 96.0f, 0.0f, 1.0f);
    return {2.7f + 14.1f * elevation, 8.0f, 0.012f + (dark ? 0.168f : 0.108f) * elevation};
}

// Gaussian survival function of the shifted signed distance. Unlike a blurred
// filled mask, this preserves the observed shared profile on small members of
// an elevated container. Maximum approximation error is below 7.5e-8.
inline constexpr const char* glassShadowCDF = R"SKSL(
float glassShadowTail(float z) {
    float t=1.0/(1.0+0.2316419*abs(z));
    float q=0.39894228*exp(-0.5*z*z)*t*(0.319381530+t*(-0.356563782+t*(1.781477937+t*(-1.821255978+t*1.330274429))));
    return z>=0 ? q : 1-q;
}
)SKSL";

inline std::string glassShadowGeometrySource() {
    const auto source = BackdropEffect::glass().shaderSource();
    const auto start = source.find("float circularBoxSDF(");
    return source.substr(start, source.find("float toBrightness", start) - start);
}

}
