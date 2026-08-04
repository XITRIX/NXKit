#include "CABackdropEffectLayer.hpp"

#include "include/core/SkRRect.h"
#include "include/core/SkM44.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"

#include <algorithm>
#include <stdexcept>
#include <string>

using namespace NXKit;

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

CABackdropEffectLayer::CABackdropEffectLayer(const BackdropEffect& effect):
    CALayer(),
    _effect(effect),
    _runtimeEffect(compile(effect))
{
    validateUniforms(_effect, *_runtimeEffect);
}

CABackdropEffectLayer::CABackdropEffectLayer(CABackdropEffectLayer* layer):
    CALayer(layer),
    _effect(layer->_effect),
    _runtimeEffect(layer->_runtimeEffect),
    _cachedImageFilter(layer->_cachedImageFilter),
    _cachedFilterSize(layer->_cachedFilterSize),
    _cachedFilterCornerRadius(layer->_cachedFilterCornerRadius),
    _cachedFilterContentsScale(layer->_cachedFilterContentsScale),
    _hasCachedFilterState(layer->_hasCachedFilterState)
{}

sk_sp<SkRuntimeEffect> CABackdropEffectLayer::compile(const BackdropEffect& effect) {
    auto result = SkRuntimeEffect::MakeForShader(SkString(effect._shaderSource.c_str()));
    if (!result.effect) {
        throw std::invalid_argument(
            "BackdropEffect SkSL failed to compile: " + std::string(result.errorText.c_str())
        );
    }
    return std::move(result.effect);
}

void CABackdropEffectLayer::validateUniforms(
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
    if (runtimeEffect.children().size() != 1) {
        throw std::invalid_argument(
            "BackdropEffect currently supports exactly one child shader"
        );
    }

    validateAutomaticUniform(runtimeEffect, "resolution", UniformType::kFloat2);
    validateAutomaticUniform(runtimeEffect, "lensCenter", UniformType::kFloat2);
    validateAutomaticUniform(runtimeEffect, "lensSize", UniformType::kFloat2);
    validateAutomaticUniform(runtimeEffect, "cornerRadius", UniformType::kFloat);
    validateAutomaticUniform(runtimeEffect, "contentScale", UniformType::kFloat);

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

void CABackdropEffectLayer::setEffect(const BackdropEffect& effect) {
    auto runtimeEffect = compile(effect);
    validateUniforms(effect, *runtimeEffect);

    _effect = effect;
    _runtimeEffect = std::move(runtimeEffect);
    invalidateImageFilterCache();
    setNeedsDisplay();
}

std::shared_ptr<CALayer> CABackdropEffectLayer::copy() {
    return new_shared<CABackdropEffectLayer>(this);
}

void CABackdropEffectLayer::invalidateImageFilterCache() {
    _cachedImageFilter.reset();
    _hasCachedFilterState = false;
}

sk_sp<SkImageFilter> CABackdropEffectLayer::imageFilterForCurrentState() {
    const auto layerSize = bounds().size;
    const auto layerCornerRadius = cornerRadius();
    const auto layerContentsScale = contentsScale();
    if (_cachedImageFilter
        && _hasCachedFilterState
        && _cachedFilterSize == layerSize
        && _cachedFilterCornerRadius == layerCornerRadius
        && _cachedFilterContentsScale == layerContentsScale)
    {
        return _cachedImageFilter;
    }

    SkRuntimeEffectBuilder builder(_runtimeEffect);
    const auto size = SkV2{layerSize.width, layerSize.height};

    if (findUniform(*_runtimeEffect, "resolution")) {
        builder.uniform("resolution") = size;
    }
    if (findUniform(*_runtimeEffect, "lensCenter")) {
        builder.uniform("lensCenter") = SkV2{size.x * 0.5f, size.y * 0.5f};
    }
    if (findUniform(*_runtimeEffect, "lensSize")) {
        builder.uniform("lensSize") = size;
    }
    if (findUniform(*_runtimeEffect, "cornerRadius")) {
        builder.uniform("cornerRadius") = std::max<NXFloat>(0, cornerRadius());
    }
    if (findUniform(*_runtimeEffect, "contentScale")) {
        builder.uniform("contentScale") = contentsScale();
    }

    for (const auto& [name, components] : _effect._uniforms) {
        builder.uniform(name).set(components.data(), static_cast<int>(components.size()));
    }

    sk_sp<SkImageFilter> input;
    if (_effect._backdropBlurRadius > 0) {
        input = SkImageFilters::Blur(
            _effect._backdropBlurRadius,
            _effect._backdropBlurRadius,
            SkTileMode::kClamp,
            nullptr
        );
    }

    auto imageFilter = SkImageFilters::RuntimeShader(
        builder,
        _effect._maximumSampleRadius,
        _effect._backdropShaderName,
        std::move(input)
    );
    if (!imageFilter) {
        invalidateImageFilterCache();
        return nullptr;
    }

    _cachedImageFilter = std::move(imageFilter);
    _cachedFilterSize = layerSize;
    _cachedFilterCornerRadius = layerCornerRadius;
    _cachedFilterContentsScale = layerContentsScale;
    _hasCachedFilterState = true;
    return _cachedImageFilter;
}

void CABackdropEffectLayer::draw(SkCanvas* context) {
    const auto layerBounds = bounds();
    if (!_runtimeEffect
        || !layerBounds.size.valid()
        || layerBounds.width() <= 0
        || layerBounds.height() <= 0)
    {
        return;
    }

    auto imageFilter = imageFilterForCurrentState();
    if (!imageFilter) return;

    const auto rect = SkRect::MakeWH(layerBounds.width(), layerBounds.height());
    const auto radius = std::clamp<NXFloat>(
        cornerRadius(),
        0,
        std::min(layerBounds.width(), layerBounds.height()) * 0.5f
    );
    SkRRect roundedRect;
    roundedRect.setRectXY(rect, radius, radius);

    SkPaint effectPaint;
    effectPaint.setAntiAlias(true);
    effectPaint.setImageFilter(std::move(imageFilter));

    context->save();
    context->clipRRect(roundedRect, true);
    SkCanvas::SaveLayerRec effectLayer(
        &rect,
        &effectPaint,
        SkCanvas::kInitWithPrevious_SaveLayerFlag
    );
    context->saveLayer(effectLayer);
    context->restore();
    context->restore();
}
