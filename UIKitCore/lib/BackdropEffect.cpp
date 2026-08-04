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
 * changes the host-side bindings and default tuning; the shader's SDF lens,
 * refraction, and dispersion are retained. Its former fixed white edge glint
 * is replaced by a narrow Fresnel-style reflection sampled from the live
 * backdrop just outside the lens.
 */

#include <BackdropEffect.h>

#include <cmath>
#include <stdexcept>
#include <utility>

using namespace NXKit;

namespace {

constexpr const char* glassShader = R"SKSL(
uniform float2 resolution;
uniform float2 lensCenter;
uniform float2 lensSize;
uniform float cornerRadius;
uniform float maximumSampleRadius;
uniform float refraction;
uniform float curve;
uniform float dispersion;
uniform float saturation;
uniform float contrast;
uniform float4 tint;
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
uniform shader content;

const float SMOOTH_EDGE_PX = 1.5;

// SkImageFilters::RuntimeShader uses maximumSampleRadius to size its backdrop
// input. Keep every dynamic child evaluation inside the same radius even when
// callers retune the built-in effect's displacement uniforms.
half4 sampleBackdrop(float2 origin, float2 sampleXY) {
    float2 offset = sampleXY - origin;
    float offsetLength = length(offset);
    float scale = min(
        1.0,
        max(maximumSampleRadius, 0.0) / max(offsetLength, 0.0001)
    );
    return content.eval(origin + offset * scale);
}

// Signed distance to a box with rounded corners.
float boxRoundedSDF(float2 p, float2 halfDim, float r) {
    float2 d = abs(p) - halfDim + float2(r);
    float exterior = length(max(d, 0.0));
    float interior = min(max(d.x, d.y), 0.0);
    return exterior + interior - r;
}

// Outward-facing direction vector from the lens surface.
float2 lensNormalDirection(float2 p, float2 halfDim, float r) {
    float2 d = abs(p) - halfDim + float2(r);
    float2 s = float2(p.x >= 0.0 ? 1.0 : -1.0, p.y >= 0.0 ? 1.0 : -1.0);

    if (max(d.x, d.y) > 0.0) {
        return s * normalize(max(d, 0.0));
    }
    return d.x > d.y ? float2(s.x, 0.0) : float2(0.0, s.y);
}

float toBrightness(half3 c) {
    return dot(c, half3(0.2126, 0.7152, 0.0722));
}

half3 processColor(half3 src, float vibrancy, float intensity, float4 overlay) {
    float mono = toBrightness(src);
    half3 vibrant = half3(clamp(mix(half3(mono), src, vibrancy), 0.0, 1.0));
    half3 adjusted = half3(clamp((vibrant - 0.5) * intensity + 0.5, 0.0, 1.0));
    return mix(adjusted, half3(overlay.rgb), overlay.a);
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

half4 main(float2 xy) {
    float2 halfDim = lensSize * 0.5;
    float r = min(cornerRadius, min(halfDim.x, halfDim.y));

    float2 p = xy - lensCenter;
    float sdf = boxRoundedSDF(p, halfDim, r);

    if (sdf > SMOOTH_EDGE_PX) {
        return content.eval(xy);
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

    pixel.rgb = processColor(pixel.rgb, saturation, contrast, tint);

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

    float alpha = 1.0 - smoothstep(-SMOOTH_EDGE_PX * 0.5, SMOOTH_EDGE_PX * 0.5, sdf);
    half4 background = sampleBackdrop(xy, xy);
    return mix(background, pixel, alpha);
}
)SKSL";

bool isAutomaticUniformName(const std::string& name) {
    return name == "resolution"
        || name == "lensCenter"
        || name == "lensSize"
        || name == "cornerRadius"
        || name == "contentScale"
        || name == "maximumSampleRadius";
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
    // Cloudy's shader models the optic separately from blur. A small backdrop
    // blur keeps the lens crisp while softening high-frequency aliasing after
    // refraction, which is closer to the current iOS glass appearance.
    BackdropEffect effect(glassShader, "content", 64, 2.5f);
    // Keep the curved depth field active across most of the lens. The former
    // 0.25 values collapsed refraction into a thin edge band, leaving the body
    // visually close to an ordinary blurred backdrop.
    effect.setUniform("refraction", 0.90f);
    effect.setUniform("curve", 0.55f);
    effect.setUniform("dispersion", 0.32f);
    effect.setUniform("saturation", 1.12f);
    effect.setUniform("contrast", 1.04f);
    effect.setUniform("tint", UIColor(255, 255, 255, 32));
    effect.setUniform("edge", 1.0f);
    effect.setUniform("specStrength", 0.52f);
    effect.setUniform("specFallbackStrength", 0.50f);
    effect.setUniform("specBrightFallbackStrength", 0.14f);
    effect.setUniform("specLightDirection", NXPoint(0.0f, -1.0f));
    effect.setUniform("specDirectionalPower", 3.0f);
    effect.setUniform("specWidthPx", 4.0f);
    effect.setUniform("reflectionSamplePx", 12.0f);
    effect.setUniform("reflectionSpreadPx", 3.0f);
    effect.setUniform("brightBackdropShade", 0.008f);
    return effect;
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
