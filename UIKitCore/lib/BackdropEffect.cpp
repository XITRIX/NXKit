/*
 * Portions of the liquid-glass shader are derived from Cloudy:
 * https://github.com/skydoves/Cloudy
 * Copyright 2022 Jaewoong Eum
 * Licensed under the Apache License, Version 2.0.
 *
 * The backdrop-colored specular reflection is adapted from the OpticalBorder
 * shader in Liquid Glass Easy:
 * https://github.com/AhmeedGamil/liquid_glass_easy
 * Copyright (c) 2025 Ahmed Gamil
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Modified for NXKit's Skia backdrop pipeline in 2026. The NXKit adaptation
 * retains the legacy custom-shader path. Built-in UIGlassEffect uses the
 * reference lens below and native UIKit calibration in Demo/GlassComparison.
 *
 * The UIKit glass material path below is adapted from liquid_glass_widgets:
 * https://github.com/sdegenaar/liquid_glass_widgets
 * Copyright 2025 Tim Lehmann for whynotmake.it
 * Copyright (c) 2024–2026 Sebastian Degenaar
 * MIT license; complete notices in licenses/liquid_glass_widgets-MIT.txt.
 * Reference: 5d2859796ef0f6964e2fdf25962b033eede27814.
 * SkSL adaptation retains live backdrop sampling in place of Flutter's
 * captured texture and uses view-local points for all optical distances.
 */

#include <BackdropEffect.h>
#include <UIGlassEffect.h>
#include "LayerCornerGeometry.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

using namespace NXKit;

namespace {

const std::string glassShader = std::string(R"SKSL(
uniform float2 resolution;
uniform float2 lensCenter;
uniform float2 lensSize;
uniform float cornerRadius;
uniform float cornerCurve;
uniform float externalShapeCoverage;
uniform float maximumSampleRadius;
uniform float refraction;
uniform float curve;
uniform float dispersion;
uniform float saturation;
uniform float contrast;
uniform float luminosity;
uniform float4 tint;
// 0: explicit optical overrides; 1: UIKit clear tint; 2: UIKit regular tint.
uniform float tintMode;
uniform float edge;
uniform float specStrength;
uniform float specFallbackStrength;
uniform float specBrightFallbackStrength;
uniform float2 specLightDirection;
uniform float specDirectionalPower;
uniform float specWidthPx;
uniform float reflectionSamplePx;
uniform float reflectionSpreadPx;
uniform float brightBackdropShade;
uniform float referenceAppearance;
uniform float bodyMode;
uniform float backdropLuma;
uniform float edgeAbsorption;
uniform float adaptiveTint;
uniform float bodyCompression;
uniform float neutralDetail;
// Resolved frost bounds the secondary kernels, including explicit zero frost.
uniform float materialFrost;
uniform float contentScale;
uniform float filterScale;
uniform float interactionIntensity;
uniform float2 interactionLocation;
uniform shader content;
uniform shader sharpContent;

const float SMOOTH_EDGE_PX = 1.5;

// SkImageFilters::RuntimeShader uses maximumSampleRadius to size its backdrop
// input. Keep every dynamic child evaluation inside the same radius even when
// callers retune the built-in effect's displacement uniforms.
float2 boundedBackdropXY(float2 origin, float2 sampleXY) {
    float2 offset = sampleXY - origin;
    float offsetLength = length(offset);
    float scale = min(
        1.0,
        max(maximumSampleRadius, 0.0) / max(offsetLength, 0.0001)
    );
    return origin + offset * scale;
}

half4 sampleBackdrop(float2 origin, float2 sampleXY) {
    return content.eval(boundedBackdropXY(origin, sampleXY) * max(filterScale, 1.0));
}

half4 sampleSharpBackdrop(float2 origin, float2 sampleXY) {
    return sharpContent.eval(boundedBackdropXY(origin, sampleXY) * max(filterScale, 1.0));
}

)SKSL") + Internal::layerCornerShaderUniforms + Internal::layerCornerShaderSource() + R"SKSL(
float toBrightness(half3 c) {
    return dot(c, half3(0.2126, 0.7152, 0.0722));
}

// Native gallery captures distinguish an opaque-color tint's vibrancy from
// an alpha overlay. Work in linear RGB for transmitted tint luminance; the
// source backdrop's sRGB luma controls how much of the supplied color survives.
float3 tintLinear(float3 color) {
    return mix(color / 12.92, pow((color + 0.055) / 1.055, float3(2.4)), step(float3(0.04045), color));
}
float3 tintSRGB(float3 color) {
    color = max(color, float3(0));
    return mix(color * 12.92, 1.055 * pow(color, float3(1.0/2.4)) - 0.055, step(float3(0.0031308), color));
}
float3 nativeTintColor(float3 color, float3 transmitted) {
    float amount = clamp(toBrightness(half3(transmitted)), 0.0, 1.0);
    float red = max(color.r - max(color.g, color.b), 0.0);
    float green = max(color.g - max(color.r, color.b), 0.0);
    float gain = 0.395 + 0.265 * red - 0.045 * green;
    float bias = -0.014 - 0.025 * red + 0.022 * green;
    if (tintMode > 1.5) {
        // Native regular tint reacts to backdrop chroma as well as luminance.
        // A luminance-only model over-brightened coral on saturated backgrounds.
        float chroma = max(transmitted.r, max(transmitted.g, transmitted.b))
            - min(transmitted.r, min(transmitted.g, transmitted.b));
        if (backdropLuma < 0.5) {
            float size = smoothstep(52.0, 120.0, min(lensSize.x, lensSize.y));
            float luma = clamp(amount - 0.15 * chroma * 4.0 * amount * (1.0 - amount), 0.0, 1.0);
            float knee = mix(0.54, 0.24, size);
            float response = (1.0 - exp(-luma / knee)) / (1.0 - exp(-1.0 / knee)) * mix(1.0, 0.61, size);
            float3 base = color - color * (1.0 - color) * 0.02;
            return base * (1.0 - 0.02 * response) + (1.0 - base) * (0.12 + 0.09 * red) * response;
        }
        amount = pow(clamp(amount - 0.263 * chroma * 4.0 * amount * (1.0 - amount), 0.0, 1.0), 1.14);
        float base = 0.4 - 0.235 * red;
        amount = base + (1.0 - base) * amount;
        gain = 0.395 + 0.50 * red - 0.045 * green;
        bias = -0.014 - 0.043 * red + 0.022 * green;
    }
    // Retain the existing clear tint transfer. Regular uses the same linear
    // RGB interpolation with the distinct response measured above.
    float3 linear = tintLinear(color);
    float3 shaded = max(linear * gain + bias, float3(0));
    return clamp(tintSRGB(mix(shaded, linear, amount)), 0.0, 1.0);
}

half3 processColor(
    half3 src,
    float vibrancy,
    float intensity,
    float luminosityAdjustment,
    float4 overlay
) {
    float mono = toBrightness(src);
    half3 vibrant = half3(clamp(mix(half3(mono), src, vibrancy), 0.0, 1.0));
    half3 adjusted = half3(clamp((vibrant - 0.5) * intensity + 0.5, 0.0, 1.0));
    half3 luminous = half3(clamp(adjusted + luminosityAdjustment, 0.0, 1.0));
    return mix(luminous, half3(overlay.rgb), overlay.a);
}

float directionalHighlight(float2 normal) {
    float directionLength = max(length(specLightDirection), 0.0001);
    float2 lightDirection = specLightDirection / directionLength;
    float highlightPower = max(specDirectionalPower, 1.0);
    float key = pow(max(dot(normal, lightDirection), 0.0), highlightPower);
    float kick = pow(max(dot(normal, -lightDirection), 0.0), highlightPower);
    return clamp(key + kick, 0.0, 1.0);
}

// Compress a backdrop sample toward a coherent reflection tone. This follows
// Liquid Glass Easy's OpticalBorder approach: preserve nearby color while
// avoiding a noisy, literal copy of every source pixel. The achromatic
// fallbacks are directionally lit instead of forming a constant border.
half3 reflectionColor(half3 backdrop, float2 normal, half3 bodyColor) {
    float luminance = toBrightness(backdrop);
    half3 saturated = backdrop / max(luminance, 0.001);
    saturated = mix(backdrop, saturated, 0.75);

    float colorfulness = length(float3(backdrop) - float3(luminance));
    float colorMix = clamp(colorfulness + 0.35, 0.35, 0.9);
    float targetBrightness = clamp(luminance * 1.2, 0.0, 1.0);
    half3 reflected = half3(clamp(
        mix(half3(targetBrightness), saturated, colorMix),
        0.0,
        1.0
    ));

    float directionality = directionalHighlight(normal);

    // A perfectly black environment has no hue or luminance to carry into the
    // rim. Start from the processed glass body so unlit sides do not become a
    // dark outline, then add directional white key and kick reflections.
    float maxChannel = max(backdrop.r, max(backdrop.g, backdrop.b));
    float blackFallback = 1.0 - smoothstep(0.0, 0.015, maxChannel);
    reflected = mix(reflected, bodyColor, blackFallback);
    reflected = mix(
        reflected,
        half3(1.0),
        blackFallback
            * directionality
            * clamp(specFallbackStrength, 0.0, 1.0)
    );

    // White reflected into white is equally invisible. On bright neutral
    // samples, shift the reflection toward grey. Keep a small ambient term so
    // the sides remain perceptible while the directional lobes stay dominant.
    float minChannel = min(backdrop.r, min(backdrop.g, backdrop.b));
    float chroma = maxChannel - minChannel;
    float brightNeutral = smoothstep(0.85, 0.98, luminance)
        * (1.0 - smoothstep(0.02, 0.12, chroma));
    float brightVisibility = 0.15 + directionality * 0.85;
    return mix(
        reflected,
        half3(0.0),
        brightNeutral
            * brightVisibility
            * clamp(specBrightFallbackStrength, 0.0, 1.0)
    );
}

half3 sampleReflection(
    float2 xy,
    float2 normal,
    float sdf,
    float2 lensCenter,
    float2 halfDim,
    half3 fallback
) {
    // Project onto the SDF boundary, continue along its normal until the ray
    // leaves the rectangular frame, then sample beyond it. This keeps rounded
    // corners from accidentally reflecting pixels that are outside the glass
    // shape but still inside its frame.
    float2 boundaryXY = xy - normal * sdf;
    float2 frameRemaining = max(
        halfDim - abs(boundaryXY - lensCenter),
        float2(0.0)
    );
    float2 absNormal = abs(normal);
    float toFrameX = absNormal.x > 0.0001
        ? frameRemaining.x / absNormal.x
        : 100000.0;
    float toFrameY = absNormal.y > 0.0001
        ? frameRemaining.y / absNormal.y
        : 100000.0;
    float toFrame = min(toFrameX, toFrameY);

    float2 tangent = float2(-normal.y, normal.x);
    float2 reflectionXY = boundaryXY
        + normal * (toFrame + max(reflectionSamplePx, 0.0));
    float spread = max(reflectionSpreadPx, 0.0);

    half4 center = sampleBackdrop(xy, reflectionXY);
    half4 before = sampleBackdrop(xy, reflectionXY - tangent * spread);
    half4 after = sampleBackdrop(xy, reflectionXY + tangent * spread);
    half4 reflected = center * 0.5 + (before + after) * 0.25;

    // At a window edge the overscan may be transparent. Fade back to the
    // refracted pixel rather than introducing a dark or transparent seam.
    return reflectionColor(
        mix(fallback, reflected.rgb, clamp(reflected.a, 0.0, 1.0)),
        normal,
        fallback
    );
}

// The reference's paraxial lens, calibrated against the native UIKit fixture
// in Demo/GlassComparison. Analytic SDF normals replace its geometry texture.
half4 referenceGlass(float2 xy, float2 halfDim, float r, float2 p, float sdf) {
    float pixelWidth = 1.0 / max(contentScale, 1.0);
    float distanceInside = max(-sdf, 0.0);
    float2 normal = lensNormalDirection(p, halfDim, r);
    float2 lightDirection = specLightDirection / max(length(specLightDirection), 0.0001);
    // A quarter-circle bevel: the XY normal goes from horizontal at the rim
    // to zero on the flat face. Fade before the rounded SDF's medial axis.
    float bevel = min(18.0 * refraction / 0.90, max(r, 0.0));
    float slope = bevel > 0.0001 ? clamp(1.0 - distanceInside / bevel, 0.0, 1.0) : 0.0;
    float normalZ = sqrt(max(0.0, 1.0 - slope * slope));

    // Regular uses a C1 quintic shoulder with zero endpoint slopes. Its softer
    // inner shoulder reduces the deep fold without adding an angular seam.
    // Keep clear's existing stronger paraxial displacement.
    float reach = min(2.0 * min(halfDim.x, halfDim.y), max(maximumSampleRadius, 0.0));
    float u = bevel > 0.0001 ? clamp(distanceInside / bevel, 0.0, 1.0) : 1.0;
    float transfer = bodyMode > 0.5 ? slope*slope : (1.0-u*u*(3.0-2.0*u))*(1.0-u*u);
    float displacement = min((bodyMode > 0.5 ? 40.0 : 20.0) * (curve / 0.55) * transfer, reach);
    float2 sampleXY = xy - normal * displacement;
    float2 chromaticShift = normal * displacement * dispersion * 0.3;
    // Glass carries its frosted material through the entire bevel.
    // Fading frost out with the surface normal exposes a broad transparent
    // perimeter, unlike UIKit. Clear uses a much smaller blur kernel.
    half4 pixel = sampleBackdrop(xy, sampleXY);
    half3 frostedRGB = pixel.rgb;
    half4 sharpPixel = pixel;
    if (dispersion > 0.0 && slope > 0.0) {
        half4 red = sampleBackdrop(xy, sampleXY + chromaticShift);
        half4 blue = sampleBackdrop(xy, sampleXY - chromaticShift);
        pixel = half4(red.r, pixel.g, blue.b, pixel.a);
    }
    // Preserve the chromatic assembly above; unshifted detail vanishes at frost0.
    if (neutralDetail > 0.0) {
        float smallDetail = 1.0 - smoothstep(64.0, 160.0, min(lensSize.x,lensSize.y));
        sharpPixel = sampleSharpBackdrop(xy, sampleXY);
        // A fully sharp high-pass creates a needle through the frost on larger
        // controls. A separable tent preserves the smoother native detail lane.
        float detailRadius = min(materialFrost,
            4.5 * smoothstep(80.0,160.0,min(lensSize.x,lensSize.y)));
        half4 softDetail = sharpPixel;
        if (detailRadius > 0.0 && smallDetail > 0.0) {
            softDetail = sharpPixel*0.25;
            softDetail += (sampleSharpBackdrop(xy,sampleXY+float2(detailRadius,0))
                +sampleSharpBackdrop(xy,sampleXY-float2(detailRadius,0))
                +sampleSharpBackdrop(xy,sampleXY+float2(0,detailRadius))
                +sampleSharpBackdrop(xy,sampleXY-float2(0,detailRadius)))*0.125;
            softDetail += (sampleSharpBackdrop(xy,sampleXY+float2(detailRadius,detailRadius))
                +sampleSharpBackdrop(xy,sampleXY-float2(detailRadius,detailRadius))
                +sampleSharpBackdrop(xy,sampleXY+float2(detailRadius,-detailRadius))
                +sampleSharpBackdrop(xy,sampleXY-float2(detailRadius,-detailRadius)))*0.0625;
        }
        half3 detail = softDetail.rgb - frostedRGB;
        detail = backdropLuma > 0.5 ? max(detail,half3(0)) : min(detail,half3(0));
        pixel.rgb += detail * smallDetail * neutralDetail;
    }
    // The light regular material filters chroma over a wider context than luma. A
    // positive five-tap kernel avoids the phase inversion of a bare tap ring.
    // Preserve the transmitted luminance, coverage, uniform colors and clear.
    if (bodyMode < 0.5 && backdropLuma > 0.5 && materialFrost > 0.0) {
        float radius = min(20.0, materialFrost * 8.0);
        half4 wider = (sampleBackdrop(xy,sampleXY+float2(radius,0))
            +sampleBackdrop(xy,sampleXY-float2(radius,0))
            +sampleBackdrop(xy,sampleXY+float2(0,radius))
            +sampleBackdrop(xy,sampleXY-float2(0,radius)))*0.25;
        half3 centerRGB = pixel.rgb/max(pixel.a,0.0001);
        half3 widerRGB = wider.rgb/max(wider.a,0.0001);
        half3 chromaDifference = widerRGB-centerRGB;
        chromaDifference -= half3(toBrightness(chromaDifference));
        pixel.rgb += chromaDifference*pixel.a*0.5;
    }
    half4 background = sampleSharpBackdrop(xy, xy);
    if (pixel.a <= 0.0) pixel = background;
    pixel.rgb /= max(pixel.a, 0.0001);
    float transmittedLuma = toBrightness(pixel.rgb);
    // Regular vibrancy needs extended RGB: early clamping washes out saturated
    // red/green/yellow backdrops before the neutral material is composited.
    float vibrancy = saturation;
    if (neutralDetail > 0.0 && saturation > 1.0) {
        // Blur mixes neighboring hues. Limit only the excess vibrancy by the
        // chroma that survives that mixing, preserving solid colors/gradients.
        // Reuse the sharp sample already needed by the neutral detail path.
        half3 sharpRGB = sharpPixel.rgb / max(sharpPixel.a, 0.0001);
        float sharpChroma = max(sharpRGB.r, max(sharpRGB.g, sharpRGB.b))
            - min(sharpRGB.r, min(sharpRGB.g, sharpRGB.b));
        float bodyChroma = max(pixel.r, max(pixel.g, pixel.b)) - min(pixel.r, min(pixel.g, pixel.b));
        vibrancy = 1.0 + (saturation - 1.0) * clamp(bodyChroma / max(sharpChroma, 0.0001), 0.0, 1.0);
    }
    half3 body = mix(half3(transmittedLuma), pixel.rgb, vibrancy);
    if (bodyMode > 0.5) body = clamp(body,0.0,1.0);
    body = bodyMode > 0.5
        ? half3(clamp((body - 0.5) * contrast + 0.5 + luminosity, 0.0, 1.0))
        : half3(body * contrast + luminosity);

    // UIKit's small regular material responds strongly to the surrounding
    // content: nearly clear over black, but a neutral veil over busy artwork.
    // These uniform sample locations are inside the output lens rectangle,
    // already covered by the backdrop input; they do not need displacement
    // overscan. A common tone across the lens avoids halos around source detail.
    float contextLuma = 0.0;
    float tintAlpha = tint.a;
    if (adaptiveTint > 0.0) {
        float2 span = halfDim * 0.6;
        float scale = max(filterScale, 1.0);
        contextLuma = toBrightness(content.eval(lensCenter * scale).rgb);
        contextLuma += toBrightness(content.eval((lensCenter + float2(-span.x, -span.y)) * scale).rgb);
        contextLuma += toBrightness(content.eval((lensCenter + float2(span.x, -span.y)) * scale).rgb);
        contextLuma += toBrightness(content.eval((lensCenter + float2(-span.x, span.y)) * scale).rgb);
        contextLuma += toBrightness(content.eval((lensCenter + span) * scale).rgb);
        contextLuma *= 0.2;
        tintAlpha = clamp(tint.a + adaptiveTint * (1.0 - exp(-contextLuma / 0.11)), 0.0, 1.0);
    }

    // Regular material keeps a visible body; chromatic tint changes hue without
    // losing backdrop luminance. Clear material uses the direct tint path.
    half3 directTint = mix(body, half3(tint.rgb), tintAlpha);
    half3 lumaTint = half3(clamp(tint.rgb + toBrightness(body) - toBrightness(half3(tint.rgb)), 0.0, 1.0));
    float chroma = max(tint.r, max(tint.g, tint.b)) - min(tint.r, min(tint.g, tint.b));
    body = tintMode > 0.5
        ? mix(body, half3(nativeTintColor(tint.rgb, pixel.rgb)), tintAlpha)
        : (bodyMode > 0.5 ? directTint
            : mix(directTint, mix(body, lumaTint, tintAlpha), clamp(chroma * 8.0, 0.0, 1.0)));
    float bodyLuma = clamp(toBrightness(body), 0.0, 1.0);
    if (bodyMode < 0.5 && backdropLuma < 0.5 && tintMode < 0.5) body *= 1.0 - bodyCompression * bodyLuma * bodyLuma;
    body = clamp(body,0.0,1.0);
    bodyLuma = clamp(toBrightness(body),0.0,1.0);

    float along = dot(normal, lightDirection);
    float rimThickness = 1.0 - sqrt(max(0.0, 1.0 - (1.0 - normalZ) * (1.0 - normalZ)));
    float absorption = sqrt(max(rimThickness, 0.0)) * edgeAbsorption * mix(1.4, 0.6, along * 0.5 + 0.5);
    half3 result = body * max(0.0, 1.0 - absorption);

    // Directional regular reflection: no omnidirectional white hairline.
    // Preserve the existing clear rim separately.
    if (bodyMode < 0.5) {
        float width = max(specWidthPx / 0.8, 0.0001);
        float inset = max(0.0, distanceInside / width - 0.55);
        float lobe = pow(abs(along), max(specDirectionalPower, 1.0));
        float rimLight = lobe * pow(max(1.0 - bodyLuma, 0.0), 0.3)
            * (0.1539 * exp(-inset / 0.25) + 0.0569 * exp(-inset / 2.5))
            * (specStrength / 0.52) * edge
            * mix(specFallbackStrength, specBrightFallbackStrength, bodyLuma) / 0.52;
        if (specWidthPx <= 0.0) rimLight = 0.0;
        // Pale light-mode material still has a tight white reflection beside
        // its exterior dark contour. The contrast-scaled lobe alone vanishes
        // as the body approaches white; retain this measured near-edge core.
        rimLight += lobe * smoothstep(0.80,0.98,bodyLuma)
            * smoothstep(0.5,0.85,backdropLuma) * 0.03 * exp(-distanceInside/0.6)
            * (specStrength/0.52) * edge * (specWidthPx>0.0 ? 1.0 : 0.0);
        result += half3(rimLight);
    } else {
    // Analytic coverage of a sub-point hairline. The AA transition is centred
    // on its inner boundary instead of adding a whole extra pixel of width.
    float hairline = clamp((max(specWidthPx, 0.0) - distanceInside) / pixelWidth + 0.5, 0.0, 1.0);
    if (specWidthPx <= 0.0) hairline = 0.0;
    hairline *= edge;

    // Two localized lobes, brightest immediately inside the dark hairline.
    // No constant white ring or all-around Fresnel stripe.
    float power = max(specDirectionalPower, 1.0);
    float lobe = pow(max(along, 0.0), power) + 0.85 * pow(max(-along, 0.0), power);
    float inset = max(0.0, distanceInside - 0.75);
    float core = exp(-inset / 0.5);
    float tail = exp(-inset / (8.0 / 3.0));
    float rimLight = lobe * pow(max(1.0 - bodyLuma, 0.0), 0.3)
        * (0.175 * core + 0.035 * tail) * (specStrength / 0.52) * edge * (1.0 - hairline);
    if (rimLight > 0.0001) {
        half3 reflected = sampleReflection(xy, normal, sdf, lensCenter, halfDim, body);
        result += (half3(0.7) + reflected * 0.3) * rimLight;
    }

    // The native iPadOS 26.4.1 capture has a sub-point white reflection over
    // the material, including over black. Sampling the raw scene here made
    // regular edges transparent and produced a dark outline over white.
    float lineStrength = clamp(specFallbackStrength + contextLuma * 0.5, 0.0, 1.0);
    result = mix(result, half3(1.0), hairline * lineStrength);
    }
    result *= 1.0 - smoothstep(0.85, 0.98, toBrightness(result)) * brightBackdropShade;

    // A single press overlay: stacking the fallback's additive glow and the
    // native button overlay together produced a milky white plate on touch.
    float glow = clamp(interactionIntensity, 0.0, 1.0);
    float glowRadius = max(1.0, min(lensSize.x, lensSize.y) * 1.6);
    float sheen = 1.0 - smoothstep(0.0, glowRadius, length(xy - interactionLocation));
    // Native held-minus-settled samples separate regular neutral and tinted
    // responses: the neutral total white opacity is about .50/.44 in light/
    // dark, while explicit regular tints use .30 ambient plus the local sheen.
    // Clear retains its existing response.
    float ambientPressOpacity = bodyMode > 0.5 ? mix(0.14, 0.22, backdropLuma)
        : (tintMode > 1.5 ? 0.30 : mix(0.385, 0.47, backdropLuma));
    half3 restingResult = result;
    result = mix(result, half3(1.0), glow * ambientPressOpacity);
    result = mix(result, half3(1.0), glow * sheen * mix(0.07, 0.10, backdropLuma));
    // Long neutral platters brighten transmitted color instead of painting
    // the button's white press veil over the entire surface. Native four-tab
    // captures in both themes retain blue/brown chroma; over black the dark
    // interior rises from32 to44, rather than32 to121. Keep short buttons,
    // explicit tints and clear glass on their independently calibrated path.
    float shortSide = max(1.0, min(lensSize.x,lensSize.y));
    float longSide = max(lensSize.x,lensSize.y);
    float platterPress = (bodyMode < 0.5 && tintMode < 0.5 ? 1.0 : 0.0)
        * smoothstep(160.0,280.0,longSide) * smoothstep(2.5,4.0,longSide/shortSide);
    float peak = max(restingResult.r,max(restingResult.g,restingResult.b));
    float chromaticRange = peak-min(restingResult.r,min(restingResult.g,restingResult.b));
    float neutralPress = 1.0-smoothstep(0.01,0.10,chromaticRange);
    float lightAppearance = smoothstep(0.15,0.85,backdropLuma);
    float pressGain = mix(1.18-0.06*peak,1.06,lightAppearance);
    half3 colorPress = mix(half3(toBrightness(restingResult)),restingResult,mix(1.03,1.06,lightAppearance))*pressGain;
    colorPress += half3(neutralPress*mix(7.0/255.0,11.5/255.0,lightAppearance));
    result = mix(result,mix(restingResult,colorPress,glow),platterPress);
    float alpha = externalShapeCoverage > 0.5 ? 1.0 : clamp(0.5 - sdf / pixelWidth, 0.0, 1.0);
    return mix(background, half4(clamp(result, 0.0, 1.0) * pixel.a, pixel.a), alpha);
}

half4 main(float2 filterXY) {
    float2 xy = filterXY / max(filterScale, 1.0);
    float2 halfDim = lensSize * 0.5;
    float r = min(cornerRadius, min(halfDim.x, halfDim.y));

    float2 p = xy - lensCenter;
    float sdf = boxRoundedSDF(p, halfDim, r);

    if (sdf > SMOOTH_EDGE_PX) {
        return content.eval(filterXY);
    }

    if (referenceAppearance > 0.5) {
        return referenceGlass(xy, halfDim, r, p, sdf);
    }

    float2 normal = lensNormalDirection(p, halfDim, r);

    // Normal-based refraction through the rounded SDF lens.
    float2 sampleXY = xy;
    if (refraction > 0.0 && curve > 0.0) {
        float minDim = min(halfDim.x, halfDim.y);
        // A rounded-rectangle SDF has diagonal medial axes where its nearest
        // edge changes. Refraction must fade out before reaching those axes or
        // a large lens exposes the normal discontinuity as a sharp diagonal.
        // Corner radius is exactly the safe bevel depth; capsules retain the
        // full requested depth because r equals their shortest half-dimension.
        float requestedDepth = minDim * refraction;
        float safeDepth = max(min(requestedDepth, max(r, 1.0)), 1.0);
        float depth = clamp(-sdf / safeDepth, 0.0, 1.0);
        float curvature = 1.0 - depth;
        float bend = 1.0 - sqrt(1.0 - curvature * curvature);
        sampleXY = xy - bend * curve * minDim * normal;
    }

    // Sample each wavelength at a slightly different position for dispersion.
    half4 pixel;
    if (dispersion > 0.0) {
        float2 normP = p / halfDim;
        float2 shift = dispersion * normP * normP * normP * min(halfDim.x, halfDim.y) * 0.1;

        float2 xyR = sampleXY - shift;
        float2 xyG = sampleXY;
        float2 xyB = sampleXY + shift;

        float sdfR = boxRoundedSDF(xyR - lensCenter, halfDim, r);
        float sdfB = boxRoundedSDF(xyB - lensCenter, halfDim, r);

        half4 gVal = sampleBackdrop(xy, xyG);
        half4 rVal = (sdfR <= 0.0) ? sampleBackdrop(xy, xyR) : gVal;
        half4 bVal = (sdfB <= 0.0) ? sampleBackdrop(xy, xyB) : gVal;

        pixel = half4(rVal.r, gVal.g, bVal.b, gVal.a);
    } else {
        pixel = sampleBackdrop(xy, sampleXY);
    }

    if (pixel.a <= 0.0) {
        pixel = sampleBackdrop(xy, xy);
    }

    pixel.rgb = processColor(pixel.rgb, saturation, contrast, luminosity, tint);

    // Preserve a faint body on white backdrops. White tint alone cannot do
    // this because compositing white over white is an identity operation.
    float brightBody = smoothstep(0.85, 0.98, toBrightness(pixel.rgb));
    pixel.rgb *= 1.0 - brightBody * clamp(brightBackdropShade, 0.0, 0.1);

    // A grazing-angle reflection replaces the old fixed white border light.
    // Its color comes from live pixels just beyond the lens boundary, so nearby
    // content moves through the rim as the glass or its backdrop moves.
    if (edge > 0.0 && specStrength > 0.0) {
        float rimBand = smoothstep(-max(specWidthPx, 1.0), 0.0, sdf);
        // Avoid evaluating distant reflection coordinates for body pixels
        // whose Fresnel contribution is exactly zero.
        if (rimBand > 0.0) {
            float fresnel = rimBand * rimBand;
            float reflectance = clamp(
                fresnel * specStrength * clamp(edge, 0.0, 1.0),
                0.0,
                0.9
            );
            half3 reflected = sampleReflection(
                xy,
                normal,
                sdf,
                lensCenter,
                halfDim,
                pixel.rgb
            );
            pixel.rgb = mix(pixel.rgb, reflected, reflectance);
        }
    }

    float alpha = externalShapeCoverage > 0.5 ? 1.0
        : 1.0 - smoothstep(-SMOOTH_EDGE_PX * 0.5, SMOOTH_EDGE_PX * 0.5, sdf);
    half4 background = sampleBackdrop(xy, xy);
    return mix(background, pixel, alpha);
}
)SKSL";

bool isAutomaticUniformName(const std::string& name) {
    return name == "resolution"
        || name == "lensCenter"
        || name == "lensSize"
        || name == "cornerRadius"
        || name == "cornerCurve"
        || name == "cornerTop" || name == "cornerTopCircle"
        || name == "cornerRight" || name == "cornerRightCircle"
        || name == "externalShapeCoverage"
        || name == "contentScale"
        || name == "filterScale"
        || name == "maximumSampleRadius"
        || name == "interactionIntensity"
        || name == "interactionLocation";
}

void validateName(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("BackdropEffect uniform name cannot be empty");
    }
    if (isAutomaticUniformName(name)) {
        throw std::invalid_argument(
            "BackdropEffect uniform '" + name + "' is populated automatically"
        );
    }
}

}

BackdropEffect::BackdropEffect(
    std::string shaderSource,
    std::string backdropShaderName,
    NXFloat maximumSampleRadius,
    NXFloat backdropBlurRadius
):
    _shaderSource(std::move(shaderSource)),
    _backdropShaderName(std::move(backdropShaderName)),
    _maximumSampleRadius(maximumSampleRadius),
    _backdropBlurRadius(backdropBlurRadius)
{
    if (_shaderSource.empty()) {
        throw std::invalid_argument("BackdropEffect shader source cannot be empty");
    }
    if (_backdropShaderName.empty()) {
        throw std::invalid_argument("BackdropEffect backdrop shader name cannot be empty");
    }
    if (!std::isfinite(_maximumSampleRadius) || _maximumSampleRadius < 0) {
        throw std::invalid_argument(
            "BackdropEffect maximum sample radius must be finite and non-negative"
        );
    }
    if (!std::isfinite(_backdropBlurRadius) || _backdropBlurRadius < 0) {
        throw std::invalid_argument(
            "BackdropEffect blur radius must be finite and non-negative"
        );
    }
}

BackdropEffect BackdropEffect::glass() {
    // The low-level factory shares the clear material. Install UIGlassEffect
    // on a view for adaptive regular styling and opt-in touch feedback.
    return UIGlassEffect(UIGlassEffect::Style::clear)._backdropEffect(
        NXSize(),
        UITraitCollection::current()
    );
}

BackdropEffect BackdropEffect::glassShaderEffect(
    NXFloat maximumSampleRadius,
    NXFloat backdropBlurRadius
) {
    auto effect = BackdropEffect(
        glassShader,
        "content",
        maximumSampleRadius,
        backdropBlurRadius
    );
    effect.setUniform("materialFrost", backdropBlurRadius);
    return effect;
}

std::optional<std::vector<NXFloat>> BackdropEffect::uniform(
    const std::string& name
) const {
    const auto iterator = _uniforms.find(name);
    if (iterator == _uniforms.end()) return std::nullopt;
    return iterator->second;
}

bool BackdropEffect::isAutomaticUniform(const std::string& name) {
    return isAutomaticUniformName(name);
}

void BackdropEffect::setFloatUniform(const std::string& name, std::vector<NXFloat> value) {
    validateName(name);
    if (value.empty()) {
        throw std::invalid_argument("BackdropEffect uniform values cannot be empty");
    }
    for (const auto component : value) {
        if (!std::isfinite(component)) {
            throw std::invalid_argument("BackdropEffect uniform values must be finite");
        }
    }
    _uniforms[name] = std::move(value);
}

void BackdropEffect::setUniform(const std::string& name, NXFloat value) {
    setFloatUniform(name, {value});
}

void BackdropEffect::setUniform(const std::string& name, NXPoint value) {
    setFloatUniform(name, {value.x, value.y});
}

void BackdropEffect::setUniform(const std::string& name, NXSize value) {
    setFloatUniform(name, {value.width, value.height});
}

void BackdropEffect::setUniform(const std::string& name, const std::array<NXFloat, 3>& value) {
    setFloatUniform(name, std::vector<NXFloat>(value.begin(), value.end()));
}

void BackdropEffect::setUniform(const std::string& name, const std::array<NXFloat, 4>& value) {
    setFloatUniform(name, std::vector<NXFloat>(value.begin(), value.end()));
}

void BackdropEffect::setUniform(const std::string& name, const std::vector<NXFloat>& value) {
    setFloatUniform(name, value);
}

void BackdropEffect::setUniform(const std::string& name, const UIColor& value) {
    setFloatUniform(name, {
        static_cast<NXFloat>(value.r()) / 255.0f,
        static_cast<NXFloat>(value.g()) / 255.0f,
        static_cast<NXFloat>(value.b()) / 255.0f,
        static_cast<NXFloat>(value.a()) / 255.0f
    });
}

void BackdropEffect::removeUniform(const std::string& name) {
    if (isAutomaticUniform(name)) return;
    _uniforms.erase(name);
}
