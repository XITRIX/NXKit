#include "CABackdropEffectLayer.hpp"
#include "CAGlassContainerLayer.hpp"
#include "GlassAppearance.hpp"
#include "GlassBackdrop.hpp"
#include "LayerCornerGeometry.hpp"
#include <DispatchQueue.h>
#include <SkiaCtx.h>
#include "include/core/SkSurface.h"

#include "include/core/SkRRect.h"
#include "include/core/SkM44.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkBlurTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <string_view>

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

struct CABackdropEffectLayer::AppearanceObservation {
    std::function<void(std::optional<NXFloat>, NXSize)> observer;
    std::function<void()> requestDisplay;
    bool pending = false;
    bool missedFrame = false;
    uint64_t generation = 0;
};

CABackdropEffectLayer::CABackdropEffectLayer(const BackdropEffect& effect):
    CALayer(),
    _effect(effect),
    _runtimeEffect(compile(effect)),
    _interaction(std::make_shared<Internal::GlassInteraction>()),
    _appearanceObservation(std::make_shared<AppearanceObservation>())
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
    _cachedFilterCornerCurve(layer->_cachedFilterCornerCurve),
    _cachedFilterContentsScale(layer->_cachedFilterContentsScale),
    _hasCachedFilterState(layer->_hasCachedFilterState),
    _cachedInteractionGlow(layer->_cachedInteractionGlow),
    _cachedInteractionLocation(layer->_cachedInteractionLocation),
    _interaction(layer->_interaction),
    _appearanceObservation(layer->_appearanceObservation),
    _glassMaterial(layer->_glassMaterial),
    _glassTraits(layer->_glassTraits)
{}

void CABackdropEffectLayer::setGlassMaterial(std::optional<UIGlassEffect> glass,
                                           std::shared_ptr<UITraitCollection> traits) {
    _glassMaterial = std::move(glass);
    _glassTraits = std::move(traits);
    invalidateImageFilterCache();
    if (const auto active = presentation()) {
        auto copy = std::static_pointer_cast<CABackdropEffectLayer>(active);
        copy->_glassMaterial = _glassMaterial;
        copy->_glassTraits = _glassTraits;
        copy->invalidateImageFilterCache();
    }
}

void CABackdropEffectLayer::setAppearanceObserver(std::function<void(std::optional<NXFloat>, NXSize)> observer) {
    ++_appearanceObservation->generation;
    _appearanceObservation->pending = false;
    _appearanceObservation->missedFrame = false;
    _appearanceObservation->observer = std::move(observer);
    const auto weak = weak_from_base<CABackdropEffectLayer>();
    _appearanceObservation->requestDisplay = [weak] {
        if (const auto layer = weak.lock()) layer->setNeedsDisplay();
    };
    setNeedsDisplay();
}

void CABackdropEffectLayer::observeBackdrop(SkCanvas* context) {
    const auto state = _appearanceObservation;
    const auto size = bounds().size;
    if (!state->observer) return;
    if (state->pending) {
        state->missedFrame = true;
        return;
    }
    const auto generation = state->generation;
    if (!Internal::isSmallGlass(size)) {
        state->pending = true;
        DispatchQueue::main()->async([state, generation, size] {
            if (state->generation != generation) return;
            state->pending = false;
            if (state->observer) state->observer(std::nullopt, size);
            if (state->missedFrame && state->requestDisplay) state->requestDisplay();
            state->missedFrame = false;
        });
        return;
    }
    auto* surface = context->getSurface();
    if (!surface) return;
    const auto area = context->getTotalMatrix().mapRect(SkRect::MakeWH(size.width, size.height));
    if (!area.isFinite()) return;
    SkIRect pixels;
    area.roundOut(&pixels);
    if (!pixels.intersect(SkIRect::MakeWH(surface->width(), surface->height()))) return;
    auto image = surface->makeImageSnapshot(pixels);
    if (!image) return;
    state->pending = true;
    const auto complete = [state, generation, size](std::optional<NXFloat> luminance) {
        DispatchQueue::main()->async([state, generation, size, luminance] {
            if (state->generation != generation) return;
            state->pending = false;
            if (state->observer) state->observer(luminance, size);
            if (state->missedFrame && state->requestDisplay) state->requestDisplay();
            state->missedFrame = false;
        });
    };
    SkPixmap source;
    if (image->peekPixels(&source)) {
        // Match Graphite's repeated reduction instead of taking eight sparse
        // point samples that can alias stripes or checkerboards.
        using Completion = std::function<void(std::optional<NXFloat>)>;
        image->asyncRescaleAndReadPixels(
            SkImageInfo::MakeN32Premul(8, 8), SkIRect::MakeWH(image->width(), image->height()),
            SkImage::RescaleGamma::kSrc, SkImage::RescaleMode::kRepeatedLinear,
            [](void* opaque, std::unique_ptr<const SkImage::AsyncReadResult> pixels) {
                std::unique_ptr<Completion> callback(static_cast<Completion*>(opaque));
                if (!pixels) { (*callback)(std::nullopt); return; }
                const SkPixmap reduced(SkImageInfo::MakeN32Premul(8, 8),
                                       pixels->data(0), pixels->rowBytes(0));
                (*callback)(Internal::backdropLuminance(reduced));
            }, new Completion(complete)
        );
    } else if (auto renderer = SkiaCtx::main()) {
        renderer->readBackdrop(std::move(image), complete);
    } else {
        complete(std::nullopt);
    }
}

NXAffineTransform CABackdropEffectLayer::renderingTransform() const {
    return _interaction->transform(bounds().size);
}

void CABackdropEffectLayer::setGlassInteraction(const Internal::GlassInteraction& interaction) {
    const bool lightingChanged = _interaction->glow() != interaction.glow()
        || _interaction->location != interaction.location;
    const bool transformChanged = _interaction->inflation.value != interaction.inflation.value
        || _interaction->dragX.value != interaction.dragX.value
        || _interaction->dragY.value != interaction.dragY.value;
    *_interaction = interaction;
    if (lightingChanged) invalidateImageFilterCache();
    if (lightingChanged || transformChanged) setNeedsDisplay();
}

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

void CABackdropEffectLayer::setEffect(const BackdropEffect& effect) {
    const auto canReuseRuntimeEffect = _runtimeEffect
        && effect._shaderSource == _effect._shaderSource
        && effect._backdropShaderName == _effect._backdropShaderName;
    auto runtimeEffect = canReuseRuntimeEffect
        ? _runtimeEffect
        : compile(effect);
    validateUniforms(effect, *runtimeEffect);

    _effect = effect;
    _glassMaterial.reset();
    _glassTraits.reset();
    _runtimeEffect = std::move(runtimeEffect);
    invalidateImageFilterCache();
    if (const auto activePresentation = presentation()) {
        const auto backdropPresentation = std::static_pointer_cast<CABackdropEffectLayer>(activePresentation);
        backdropPresentation->_effect = _effect;
        backdropPresentation->_glassMaterial.reset();
        backdropPresentation->_glassTraits.reset();
        backdropPresentation->_runtimeEffect = _runtimeEffect;
        backdropPresentation->invalidateImageFilterCache();
    }
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
        && _cachedFilterCornerCurve == cornerCurve()
        && _cachedFilterContentsScale == layerContentsScale
        && _cachedInteractionGlow == _interaction->glow()
        && _cachedInteractionLocation == _interaction->location)
    {
        return _cachedImageFilter;
    }

    const auto effect = _glassMaterial
        ? _glassMaterial->_backdropEffect(layerSize, _glassTraits) : _effect;
    SkRuntimeEffectBuilder builder(_runtimeEffect);
    const auto size = SkV2{layerSize.width, layerSize.height};
    const auto filterScale = findUniform(*_runtimeEffect, "filterScale")
        && std::isfinite(layerContentsScale)
        ? std::max<NXFloat>(1, layerContentsScale) : 1.0f;

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
        const auto radius = cornerRadius();
        builder.uniform("cornerRadius") = std::isfinite(radius) && radius >= 0.0001f ? radius : 0.0f;
    }
    if (findUniform(*_runtimeEffect, "cornerCurve")) {
        builder.uniform("cornerCurve") = cornerCurve() == CALayerCornerCurve::continuous ? 1.0f : 0.0f;
    }
    if (findUniform(*_runtimeEffect, "externalShapeCoverage")) {
        builder.uniform("externalShapeCoverage") = 1.0f;
    }
    if (findUniform(*_runtimeEffect, "contentScale")) {
        builder.uniform("contentScale") = contentsScale();
    }
    if (findUniform(*_runtimeEffect, "filterScale")) {
        builder.uniform("filterScale") = filterScale;
    }
    if (findUniform(*_runtimeEffect, "maximumSampleRadius")) {
        builder.uniform("maximumSampleRadius") = effect._maximumSampleRadius;
    }

    for (const auto& [name, components] : effect._uniforms) {
        builder.uniform(name).set(components.data(), static_cast<int>(components.size()));
    }

    if (findUniform(*_runtimeEffect, "interactionIntensity")) {
        builder.uniform("interactionIntensity") = _interaction->glow();
    }
    if (findUniform(*_runtimeEffect, "interactionLocation")) {
        builder.uniform("interactionLocation") = SkV2{_interaction->location.x, _interaction->location.y};
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
    if (_runtimeEffect->findChild("sharpContent") && effect._backdropShaderName != "sharpContent") {
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
    if (!imageFilter) {
        invalidateImageFilterCache();
        return nullptr;
    }

    _cachedImageFilter = std::move(imageFilter);
    _cachedFilterSize = layerSize;
    _cachedFilterCornerRadius = layerCornerRadius;
    _cachedFilterCornerCurve = cornerCurve();
    _cachedFilterContentsScale = layerContentsScale;
    _cachedInteractionGlow = _interaction->glow();
    _cachedInteractionLocation = _interaction->location;
    _hasCachedFilterState = true;
    return _cachedImageFilter;
}

void CABackdropEffectLayer::draw(SkCanvas* context) {
    if (CAGlassContainerLayer::contains(this)) return;
    const auto layerBounds = bounds();
    if (!_runtimeEffect
        || !std::isfinite(layerBounds.width())
        || !std::isfinite(layerBounds.height())
        || layerBounds.width() <= 0
        || layerBounds.height() <= 0)
    {
        return;
    }

    observeBackdrop(context);
    auto imageFilter = imageFilterForCurrentState();
    if (!imageFilter) return;

    const auto rect = SkRect::MakeWH(layerBounds.width(), layerBounds.height());
    const auto shape = Internal::layerCornerPath(layerBounds.size, cornerRadius(), cornerCurve());

    SkPaint effectPaint;
    effectPaint.setAntiAlias(true);
    effectPaint.setImageFilter(std::move(imageFilter));

    // RuntimeShader image filters force a translate-only CTM in Skia. Render
    // opt-in shaders in pixel coordinates instead of making Skia rasterize a
    // one-point grid and upscale it on Retina displays. Their filterScale
    // uniform maps optical distances back to local points inside the shader.
    const auto filterScale = findUniform(*_runtimeEffect, "filterScale")
        && std::isfinite(contentsScale())
        ? std::max<NXFloat>(1, contentsScale()) : 1.0f;
    const auto filterRect = SkRect::MakeWH(rect.width() * filterScale, rect.height() * filterScale);
    context->save();
    // Apply coverage once in device space; the built-in shader supplies the
    // fully shaded material and shares this clip's continuous silhouette.
    if (!masksToBounds()) context->clipPath(shape, true);
    context->scale(1.0f / filterScale, 1.0f / filterScale);
    SkCanvas::SaveLayerRec effectLayer(
        &filterRect,
        &effectPaint,
        SkCanvas::kInitWithPrevious_SaveLayerFlag
    );
    context->saveLayer(effectLayer);
    context->restore();
    context->restore();

    // liquid_glass_widgets' inverse-clipped elevation/contact shadows. Draw
    // after sampling the backdrop so the glass never refracts its own shadow.
    // An explicit app shadow takes precedence; shadowOpacity(0) disables it.
    const auto material = _effect.uniform("referenceAppearance");
    const auto appearance = _effect.uniform("backdropLuma");
    if (material && material->front() > 0.5f
        && appearance && appearance->front() > 0.5f
        && !shadowColor() && shadowOpacity() > 0) {
        context->save();
        context->clipPath(shape, SkClipOp::kDifference, true);
        const auto drawShadow = [&](NXFloat blur, NXFloat offset, NXFloat alpha) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(SK_ColorBLACK);
            paint.setAlphaf(alpha * shadowOpacity());
            paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, blur * 0.57735f + 0.5f));
            context->save();
            context->translate(0, offset);
            context->drawPath(shape, paint);
            context->restore();
        };
        drawShadow(8, 2, 15.0f / 255.0f);
        drawShadow(2, 1, 5.0f / 255.0f);
        context->restore();
    }
}
