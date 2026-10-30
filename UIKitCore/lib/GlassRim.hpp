#pragma once

namespace NXKit::Internal {

// Native regular-glass exterior scans on uniform sRGB backgrounds. This is a
// narrow subtractive contour, separate from the white reflection and elevation
// shadow. Its directional attenuation is essential on pale backgrounds.
inline constexpr const char* glassRimSource = R"SKSL(
float glassRimDarkening(float distance, float2 normal, float sceneLuma,
                       float dark, float2 light, float power, float width,
                       float scale, float strength, float shortSide) {
    // Containers supply device scale times the active owner's local metric;
    // compression can legitimately make that product smaller than one.
    float pixel = 1.0 / max(scale, 0.0001);
    float band = clamp((max(width,0.0)*0.625-distance)/pixel+0.5,0.0,1.0);
    if (band<=0.0 || width<=0.0 || strength<=0.0) return 0.0;
    float along = abs(dot(normal,light/max(length(light),0.0001)));
    float direction = 1.0-mix(0.70,1.0,dark)*pow(along,max(power,1.0));
    //52/120pt neutral controls determine the dark density;80pt and the
    //continuous card validate the size interpolation. Bright chromatic scenes
    //need the same extra density as white, while gray/low-luma primaries do not.
    float bright=pow(clamp((sceneLuma-0.5)*2.0,0.0,1.0),1.5);
    float large=clamp((shortSide-52.0)/68.0,0.0,1.0);
    float darkDensity=0.24+0.31*bright
        +large*(0.05+0.17*smoothstep(0.5,0.65,sceneLuma));
    float density=mix(0.25,darkDensity,dark);
    return band*direction*density*max(strength,0.0);
}
)SKSL";

}
