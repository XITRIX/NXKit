#include "BackdropShader.hpp"
#include "LayerCornerGeometry.hpp"
#include "include/effects/SkImageFilters.h"
#include "include/core/SkM44.h"
#include "include/effects/SkRuntimeEffect.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string_view>

using namespace NXKit;
using NXKit::Internal::BackdropShader;

namespace {

using UniformType = SkRuntimeEffect::Uniform::Type;

const SkRuntimeEffect::Uniform* findUniform(
    const SkRuntimeEffect& runtimeEffect,
    const std::string& name
) {
    return runtimeEffect.findUniform(name);
}

void validateAutomaticUniform(
    const SkRuntimeEffect& runtimeEffect,
    const char* name,
    UniformType requiredType
) {
    const auto* uniform = runtimeEffect.findUniform(name);
    if (!uniform) return;
    if (uniform->type != requiredType || uniform->isArray()) {
        throw std::invalid_argument(
            std::string("BackdropEffect automatic uniform '") + name + "' has the wrong type"
        );
    }
}

bool isFloatUniform(UniformType type) {
    switch (type) {
        case UniformType::kFloat:
        case UniformType::kFloat2:
        case UniformType::kFloat3:
        case UniformType::kFloat4:
        case UniformType::kFloat2x2:
        case UniformType::kFloat3x3:
        case UniformType::kFloat4x4:
            return true;
        default:
            return false;
    }
}

}

sk_sp<SkRuntimeEffect> BackdropShader::compile(const BackdropEffect& effect) {
    auto result = SkRuntimeEffect::MakeForShader(SkString(effect._shaderSource.c_str()));
    if (!result.effect) {
        throw std::invalid_argument(
            "BackdropEffect SkSL failed to compile: " + std::string(result.errorText.c_str())
        );
    }
    return std::move(result.effect);
}

void BackdropShader::validate(
    const BackdropEffect& effect,
    const SkRuntimeEffect& runtimeEffect
) {
    const auto* child = runtimeEffect.findChild(effect._backdropShaderName);
    if (!child || child->type != SkRuntimeEffect::ChildType::kShader) {
        throw std::invalid_argument(
            "BackdropEffect shader must declare uniform shader '"
            + effect._backdropShaderName + "'"
        );
    }
    const auto* sharpChild = runtimeEffect.findChild("sharpContent");
    const bool hasSharpBackdrop = sharpChild && effect._backdropShaderName != "sharpContent";
    if (runtimeEffect.children().size() != (hasSharpBackdrop ? 2 : 1)
        || (hasSharpBackdrop && sharpChild->type != SkRuntimeEffect::ChildType::kShader)) {
        throw std::invalid_argument(
            "BackdropEffect supports one backdrop shader and an optional shader named 'sharpContent'"
        );
    }

    validateAutomaticUniform(runtimeEffect, "resolution", UniformType::kFloat2);
    validateAutomaticUniform(runtimeEffect, "lensCenter", UniformType::kFloat2);
    validateAutomaticUniform(runtimeEffect, "lensSize", UniformType::kFloat2);
    validateAutomaticUniform(runtimeEffect, "cornerRadius", UniformType::kFloat);
    validateAutomaticUniform(runtimeEffect, "cornerCurve", UniformType::kFloat);
    for (const auto* name : Internal::layerCornerShaderNames)
        validateAutomaticUniform(runtimeEffect, name, UniformType::kFloat4);
    validateAutomaticUniform(runtimeEffect, "externalShapeCoverage", UniformType::kFloat);
    validateAutomaticUniform(runtimeEffect, "contentScale", UniformType::kFloat);
    validateAutomaticUniform(runtimeEffect, "filterScale", UniformType::kFloat);
    validateAutomaticUniform(runtimeEffect, "maximumSampleRadius", UniformType::kFloat);
    validateAutomaticUniform(runtimeEffect, "interactionIntensity", UniformType::kFloat);
    validateAutomaticUniform(runtimeEffect, "interactionLocation", UniformType::kFloat2);

    for (const auto& [name, components] : effect._uniforms) {
        const auto* uniform = findUniform(runtimeEffect, name);
        if (!uniform) {
            throw std::invalid_argument(
                "BackdropEffect shader does not declare uniform '" + name + "'"
            );
        }

        if (!isFloatUniform(uniform->type)
            || uniform->sizeInBytes() != components.size() * sizeof(NXFloat))
        {
            throw std::invalid_argument(
                "BackdropEffect uniform '" + name + "' has the wrong number or type of values"
            );
        }
    }
}

std::span<const NXFloat> BackdropShader::uniformValues(const BackdropEffect& effect, const std::string& name) {
    const auto found = effect._uniforms.find(name);
    return found == effect._uniforms.end() ? std::span<const NXFloat>() : found->second;
}

sk_sp<SkImageFilter> BackdropShader::makeFilter(
    const BackdropEffect& effect, const sk_sp<SkRuntimeEffect>& runtimeEffect,
    NXSize layerSize, NXFloat radius, CALayerCornerCurve curve, NXFloat scale,
    NXFloat intensity, NXPoint location
) {
    SkRuntimeEffectBuilder builder(runtimeEffect);
    const auto size = SkV2{layerSize.width, layerSize.height};
    const auto filterScale = findUniform(*runtimeEffect, "filterScale")
        && std::isfinite(scale)
        ? std::max<NXFloat>(1, scale) : 1.0f;

    if (findUniform(*runtimeEffect, "resolution")) {
        builder.uniform("resolution") = size;
    }
    if (findUniform(*runtimeEffect, "lensCenter")) {
        builder.uniform("lensCenter") = SkV2{size.x * 0.5f, size.y * 0.5f};
    }
    if (findUniform(*runtimeEffect, "lensSize")) {
        builder.uniform("lensSize") = size;
    }
    if (findUniform(*runtimeEffect, "cornerRadius")) {
        builder.uniform("cornerRadius") = std::isfinite(radius) && radius >= 0.0001f ? radius : 0.0f;
    }
    if (findUniform(*runtimeEffect, "cornerCurve")) {
        builder.uniform("cornerCurve") = curve == CALayerCornerCurve::continuous ? 1.0f : 0.0f;
    }
    Internal::bindLayerCornerGeometry(builder, *runtimeEffect, layerSize, radius);
    if (findUniform(*runtimeEffect, "externalShapeCoverage")) {
        builder.uniform("externalShapeCoverage") = 1.0f;
    }
    if (findUniform(*runtimeEffect, "contentScale")) {
        builder.uniform("contentScale") = scale;
    }
    if (findUniform(*runtimeEffect, "filterScale")) {
        builder.uniform("filterScale") = filterScale;
    }
    if (findUniform(*runtimeEffect, "maximumSampleRadius")) {
        builder.uniform("maximumSampleRadius") = effect._maximumSampleRadius;
    }

    for (const auto& [name, components] : effect._uniforms) {
        builder.uniform(name).set(components.data(), static_cast<int>(components.size()));
    }

    if (findUniform(*runtimeEffect, "interactionIntensity")) {
        builder.uniform("interactionIntensity") = intensity;
    }
    if (findUniform(*runtimeEffect, "interactionLocation")) {
        builder.uniform("interactionLocation") = SkV2{location.x, location.y};
    }

    sk_sp<SkImageFilter> input;
    if (effect._backdropBlurRadius > 0) {
        input = SkImageFilters::Blur(
            effect._backdropBlurRadius * filterScale,
            effect._backdropBlurRadius * filterScale,
            SkTileMode::kClamp,
            nullptr
        );
    }

    sk_sp<SkImageFilter> imageFilter;
    if (runtimeEffect->findChild("sharpContent") && effect._backdropShaderName != "sharpContent") {
        // Lens folds and the half-point rim need the original scene. Blurring
        // the only input first erases those details before refraction can run.
        std::string_view children[] = {effect._backdropShaderName, "sharpContent"};
        const sk_sp<SkImageFilter> inputs[] = {std::move(input), nullptr};
        imageFilter = SkImageFilters::RuntimeShader(
            builder, effect._maximumSampleRadius * filterScale, children, inputs, 2
        );
    } else {
        imageFilter = SkImageFilters::RuntimeShader(
            builder, effect._maximumSampleRadius * filterScale, effect._backdropShaderName, std::move(input)
        );
    }
    return imageFilter;
}
