#include "RenderPerformance.hpp"
#include "BackdropShader.hpp"
#include "ProgressiveBlur.hpp"
#include "CABackdropEffectLayer.hpp"
#include "CAGlassContainerLayer.hpp"
#include "GlassAppearance.hpp"
#include "GlassShadow.hpp"
#include "GlassRim.hpp"
#include "GlassForeground.hpp"
#include "BackdropSampling.hpp"
#include "LayerCornerGeometry.hpp"
#include <DispatchQueue.h>
#include <SkiaCtx.h>
#include "include/core/SkSurface.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDevice.h"
#include "src/core/SkSpecialImage.h"

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
bool hasNativeExterior(const BackdropEffect& material, NXFloat shadowOpacity, bool explicitShadow) {
    if(material.outputOutset()<=0) return false;
    const auto highlight=material.uniform("nativeFilterHighlight");
    const auto highlightColor=material.uniform("nativeFilterHighlightColor");
    // The filter highlight occupies -height <= sdf + offset <= 0. A
    // negative offset therefore places part of its band outside the host.
    if(highlight && highlight->size()==4 && highlight->at(0)>0 && highlight->at(2)>0
       && highlightColor && highlightColor->size()==3 && highlightColor->at(0)<0) return true;
    if(explicitShadow || shadowOpacity<=0) return false;
    for(const auto* name:{"nativeShadowShape","nativeShadowResponse"}) {
        const auto value=material.uniform(name);
        if(value && value->size()==4 && value->at(3)>0) return true;
    }
    return false;
}
}


namespace {
const SkRuntimeEffect::Uniform* findUniform(const SkRuntimeEffect& effect, const char* name) {
    return effect.findUniform(name);
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
    _effect(std::make_shared<const BackdropEffect>(effect)),
    _runtimeEffect(compile(effect)),
    _interaction(std::make_shared<Internal::GlassInteraction>()),
    _appearanceObservation(std::make_shared<AppearanceObservation>())
{
    validateUniforms(*_effect, *_runtimeEffect);
}

CABackdropEffectLayer::CABackdropEffectLayer(CABackdropEffectLayer* layer):
    CALayer(layer),
    _resolvedMaterials(layer->_resolvedMaterials),
    _effect(layer->_effect),
    _variableBlur(layer->_variableBlur),
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
    // A content mask or opacity group may have an offscreen device. The
    // canvas' root surface omits siblings drawn into that current group.
    auto* device = SkCanvasPriv::TopDevice(context);
    if (!device) return;
    const auto toDevice = SkMatrix::Concat(device->globalToDevice().asM33(), context->getTotalMatrix());
    const auto area = toDevice.mapRect(SkRect::MakeWH(size.width, size.height));
    if (!area.isFinite() || area.isEmpty()) return;
    SkIRect pixels;
    area.roundOut(&pixels);
    if (!pixels.intersect(device->bounds()) || !pixels.intersect(device->devClipBounds())) return;
    // Freeze the prefix before drawing the glass. Graphite enqueues a texture
    // copy here; the existing eight-pixel async readback never waits for GPU work.
    const auto snapshot = device->snapSpecial(pixels, true);
    if (!snapshot) return;
    auto image = snapshot->asImage();
    if (!image) return;
    const auto subset = snapshot->subset();
    if (subset != image->bounds()) {
        // Graphite's internal snapshots may use an approximate backing texture.
        // Crop its valid subset on the same backend so padding never influences
        // luminance; do not rasterize or synchronously read a GPU image.
        auto cropped = device->makeSurface(image->imageInfo().makeDimensions(subset.size()), device->surfaceProps());
        if (!cropped) return;
        cropped->getCanvas()->drawImageRect(image, SkRect::Make(subset),
            SkRect::MakeWH(subset.width(), subset.height()), SkSamplingOptions(), nullptr,
            SkCanvas::kStrict_SrcRectConstraint);
        image = cropped->makeImageSnapshot();
        if (!image) return;
    }
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
    return Internal::BackdropShader::compile(effect);
}

void CABackdropEffectLayer::validateUniforms(const BackdropEffect& effect,
                                           const SkRuntimeEffect& runtimeEffect) {
    Internal::BackdropShader::validate(effect, runtimeEffect);
}

void CABackdropEffectLayer::setEffect(const BackdropEffect& effect) {
    const auto canReuseRuntimeEffect = _runtimeEffect
        && effect.shaderSource() == _effect->shaderSource()
        && effect.backdropShaderName() == _effect->backdropShaderName();
    auto runtimeEffect = canReuseRuntimeEffect
        ? _runtimeEffect
        : compile(effect);
    validateUniforms(effect, *runtimeEffect);

    _variableBlur.reset();
    _effect = std::make_shared<const BackdropEffect>(effect);
    _glassMaterial.reset();
    _glassTraits.reset();
    _runtimeEffect = std::move(runtimeEffect);
    invalidateImageFilterCache();
    if (const auto activePresentation = presentation()) {
        const auto backdropPresentation = std::static_pointer_cast<CABackdropEffectLayer>(activePresentation);
        backdropPresentation->_effect = _effect;
        backdropPresentation->_variableBlur.reset();
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

void CABackdropEffectLayer::setEffect(const UIBlurEffect& effect) {
    setEffect(BackdropEffect("uniform shader content; uniform float filterScale; half4 main(float2 p){return content.eval(p);}"));
    _variableBlur=effect;
    if(const auto active=presentation())std::static_pointer_cast<CABackdropEffectLayer>(active)->_variableBlur=effect;
    invalidateImageFilterCache();setNeedsDisplay();
}

void CABackdropEffectLayer::invalidateImageFilterCache() {
    _cachedImageFilter.reset();
    _hasCachedFilterState = false;
    _resolvedMaterials = {};
}

std::shared_ptr<const BackdropEffect> CABackdropEffectLayer::resolvedEffect(NXSize size) {
    if (!_glassMaterial) return _effect;
    for (const auto& material : _resolvedMaterials)
        if (material && material->size == size) return {material, &material->effect};
    auto material = std::make_shared<const ResolvedMaterial>(ResolvedMaterial{
        size, _glassMaterial->_backdropEffect(size, _glassTraits)});
    _resolvedMaterials[1] = std::move(_resolvedMaterials[0]);
    _resolvedMaterials[0] = std::move(material);
    // The alias retains the immutable description's owning cache entry even
    // if a later resize replaces that entry during this frame.
    return {_resolvedMaterials[0], &_resolvedMaterials[0]->effect};
}

sk_sp<SkImageFilter> CABackdropEffectLayer::imageFilterForCurrentState(SkCanvas* canvas) {
    const auto layerSize = bounds().size;
    const auto layerCornerRadius = cornerRadius();
    const auto layerContentsScale = contentsScale();
    const bool framePyramid=resolvedEffect(layerSize)->backdropPyramid().has_value()
        &&Internal::BackdropShader::canFreezePyramid(canvas,std::max<NXFloat>(1,layerContentsScale));
    if(!framePyramid)canvas=nullptr;
    if (!framePyramid && _cachedImageFilter
        && _hasCachedFilterState
        && _cachedFilterSize == layerSize
        && _cachedFilterCornerRadius == layerCornerRadius
        && _cachedFilterCornerCurve == cornerCurve()
        && _cachedFilterContentsScale == layerContentsScale
        && _cachedGlassShadowOpacity == shadowOpacity()
        && _cachedGlassHasExplicitShadow == shadowColor().has_value()
        && _cachedInteractionGlow == _interaction->glow()
        && _cachedInteractionLocation == _interaction->location)
    {
        return _cachedImageFilter;
    }

    const auto material = resolvedEffect(layerSize);
    std::optional<BackdropEffect> adjustedEffect;
    if (shadowColor() || shadowOpacity() != 1) {
        adjustedEffect = *material;
        for (const auto* name : {"nativeShadowShape", "nativeShadowResponse"}) {
            const auto value = adjustedEffect->uniform(name);
            if (value && value->size() == 4) {
                auto adjusted = *value;
                adjusted[3] *= shadowColor() ? 0 : std::max<NXFloat>(0, shadowOpacity());
                adjustedEffect->setUniform(name, adjusted);
            }
        }
    }
    if(material->uniform("nativeFaceBlack") && !hasNativeExterior(*material,shadowOpacity(),shadowColor().has_value())) {
        if(!adjustedEffect) adjustedEffect=*material;
        adjustedEffect->setOutputOutset(0);
    }
    const auto& effect = adjustedEffect ? *adjustedEffect : *material;
    // Native radiusLOD starts at level one when its prefiltered initial
    // reduction is active, including its nominally sharp sample. Generic
    // backdrop programs retain the complete atlas by default.
    auto imageFilter = _variableBlur ? Internal::progressiveBlurFilter(*_variableBlur,layerSize,layerContentsScale) : Internal::BackdropShader::makeFilter(
        effect, _runtimeEffect, layerSize, layerCornerRadius, cornerCurve(),
        layerContentsScale, _interaction->glow(), _interaction->location,canvas,_glassMaterial.has_value()
    );
    if (!imageFilter) {
        invalidateImageFilterCache();
        return nullptr;
    }

    // Frozen prefix/level images belong to this render only. Never retain a
    // frame-specific atlas in the layer's reusable description/filter cache.
    if(framePyramid)return imageFilter;

    _cachedImageFilter = std::move(imageFilter);
    _cachedFilterSize = layerSize;
    _cachedFilterCornerRadius = layerCornerRadius;
    _cachedFilterCornerCurve = cornerCurve();
    _cachedFilterContentsScale = layerContentsScale;
    _cachedGlassShadowOpacity = shadowOpacity();
    _cachedGlassHasExplicitShadow = shadowColor().has_value();
    _cachedInteractionGlow = _interaction->glow();
    _cachedInteractionLocation = _interaction->location;
    _hasCachedFilterState = true;
    return _cachedImageFilter;
}

void CABackdropEffectLayer::renderSublayers(SkCanvas* canvas) {
    if(!_glassMaterial || !_glassMaterial->isContentVibrancyEnabled() || sublayers().empty()) {
        CALayer::renderSublayers(canvas);return;
    }
    SkPaint paint;paint.setBlender(Internal::glassForegroundBlender(
        _glassTraits&&_glassTraits->userInterfaceStyle()==UIUserInterfaceStyle::dark));
    // Only isolate children. The material is already painted into backdrop;
    // applying this to the whole layer would blend the glass twice.
    const auto contentBounds=SkRect::MakeWH(bounds().width(),bounds().height());
    canvas->saveLayer(&contentBounds,&paint);CALayer::renderSublayers(canvas);canvas->restore();
}

void CABackdropEffectLayer::draw(SkCanvas* context) {
    auto* metrics = Internal::RenderPerformance::current;
    if (metrics) ++metrics->glass;
    Internal::RenderPerformanceScope timer(metrics ? &metrics->glassMs : nullptr);
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
    auto imageFilter = imageFilterForCurrentState(context);
    if (!imageFilter) return;

    const auto rect = SkRect::MakeWH(layerBounds.width(), layerBounds.height());
    const auto shape = Internal::layerCornerPath(layerBounds.size, cornerRadius(), cornerCurve());

    SkPaint effectPaint;
    effectPaint.setAntiAlias(true);
    // Variable blur replaces sampled backdrop pixels. Source-over would add
    // translucent pixels twice even for a zero-radius/transparent mask.
    if(_variableBlur)effectPaint.setBlendMode(SkBlendMode::kSrc);
    effectPaint.setImageFilter(std::move(imageFilter));

    // RuntimeShader image filters force a translate-only CTM in Skia. Render
    // opt-in shaders in pixel coordinates instead of making Skia rasterize a
    // one-point grid and upscale it on Retina displays. Their filterScale
    // uniform maps optical distances back to local points inside the shader.
    const auto filterScale = (_variableBlur || findUniform(*_runtimeEffect, "filterScale"))
        && std::isfinite(contentsScale())
        ? std::max<NXFloat>(1, contentsScale()) : 1.0f;
    auto outputRect = rect;
    const auto resolvedMaterial = resolvedEffect(layerBounds.size);
    const bool nativeMaterial = !Internal::BackdropShader::uniformValues(*resolvedMaterial, "nativeFaceBlack").empty();
    const bool nativeExterior=nativeMaterial && hasNativeExterior(*resolvedMaterial,shadowOpacity(),shadowColor().has_value());
    if (nativeExterior && !masksToBounds()) outputRect.outset(resolvedMaterial->outputOutset(), resolvedMaterial->outputOutset());
    const auto filterRect = SkRect::MakeLTRB(outputRect.left() * filterScale, outputRect.top() * filterScale,
        outputRect.right() * filterScale, outputRect.bottom() * filterScale);
    if (nativeMaterial) effectPaint.setBlendMode(SkBlendMode::kSrc);
    context->save();
    // Apply coverage once in device space; the built-in shader supplies the
    // fully shaded material and shares this clip's continuous silhouette.
    if (!masksToBounds() && !nativeExterior) context->clipPath(shape, true);
    context->scale(1.0f / filterScale, 1.0f / filterScale);
    // saveLayer's bounds are an allocation hint, not an output clip. Native
    // exterior materials replace pixels with kSrc; limit that replacement to
    // their declared output before bounding the sharp backdrop input.
    if (nativeExterior) context->clipRect(filterRect);
    SkCanvas::SaveLayerRec effectLayer(
        &filterRect,
        &effectPaint,
        SkCanvas::kInitWithPrevious_SaveLayerFlag
    );
    context->saveLayer(effectLayer);
    context->restore();
    context->restore();

    // Native materials include their own merged-distance rim and shadow in
    // the same original-backdrop pass. Never feed an old contour/shadow back
    // into that result or paint the elevation twice.
    if (nativeMaterial) return;

    // The native dark contour lies outside the material's coverage. Keep it
    // separate from the clipped body and automatic elevation, with one shared
    // distance/normal law also used by glass containers. It must never enter
    // the input sampled for the refraction above.
    const auto contourEffect = resolvedEffect(layerBounds.size);
    const auto scalar = [&](const char* name,float fallback) {
        const auto value=Internal::BackdropShader::uniformValues(*contourEffect, name);
        return !value.empty() ? value.front() : fallback;
    };
    const auto light=Internal::BackdropShader::uniformValues(*contourEffect, "specLightDirection");
    const float contourStrength=scalar("edge",0)*scalar("edgeAbsorption",0)/0.035f;
    if (scalar("referenceAppearance",0)>.5f && scalar("bodyMode",1)<.5f
        && contourStrength>0 && scalar("specWidthPx",0)>0 && light.size()==2) {
        static thread_local const auto runtime=[] {
            auto source=std::string("uniform shader scene; uniform float2 size, light; uniform float cornerRadius, cornerCurve, scale, dark, power, width, strength;\n")
                +Internal::layerCornerShaderUniforms+Internal::glassShadowGeometrySource()+Internal::glassRimSource
                +R"SKSL(
half4 main(float2 pixelXY) {
    float2 xy=pixelXY/max(scale,1.0), halfDim=size*0.5, p=xy-halfDim;
    float r=min(cornerRadius,min(halfDim.x,halfDim.y));
    float3 surface=roundedBoxSample(p,halfDim,r);
    float d=surface.x;
    float2 normal=surface.yz;
    half4 original=scene.eval(pixelXY);
    // Read beyond the already drawn material's AA footprint. Subtract only
    // the backdrop contribution: otherwise black backgrounds incorrectly lose
    // some of the glass's fractional edge coverage to the exterior contour.
    half4 outside=scene.eval((xy+normal*max(2.0-d,0.0))*max(scale,1.0));
    float luma=dot(outside.rgb/max(outside.a,0.0001),half3(0.2126,0.7152,0.0722));
    float density=glassRimDarkening(d,normal,luma,dark,light,power,width,scale,strength,min(size.x,size.y));
    half3 loss=min(half3(density)*outside.a,outside.rgb);
    return half4(max(original.rgb-loss,half3(0)),original.a);
})SKSL";
            auto compiled=SkRuntimeEffect::MakeForShader(SkString(source));
            if(!compiled.effect) throw std::runtime_error("Glass contour shader: "+std::string(compiled.errorText.c_str()));
            return compiled.effect;
        }();
        SkRuntimeEffectBuilder builder(runtime);
        const float size[]={layerBounds.width(),layerBounds.height()};
        builder.uniform("size").set(size,2);builder.uniform("light").set(light.data(),2);
        builder.uniform("cornerRadius")=cornerRadius();
        builder.uniform("cornerCurve")=cornerCurve()==CALayerCornerCurve::continuous?1.0f:0.0f;
        Internal::bindLayerCornerGeometry(builder, *runtime, layerBounds.size, cornerRadius());
        builder.uniform("scale")=filterScale;
        builder.uniform("dark")=scalar("backdropLuma",.85f)<.5f?1.0f:0.0f;
        builder.uniform("power")=scalar("specDirectionalPower",3);
        builder.uniform("width")=scalar("specWidthPx",.8f);
        builder.uniform("strength")=contourStrength;
        SkPaint paint;paint.setImageFilter(SkImageFilters::RuntimeShader(builder,3*filterScale,"scene",nullptr));
        auto contourBounds=rect;contourBounds.outset(scalar("specWidthPx",.8f)*.625f+1,scalar("specWidthPx",.8f)*.625f+1);
        context->save();context->clipPath(shape,SkClipOp::kDifference,true);
        context->scale(1/filterScale,1/filterScale);
        contourBounds=SkRect::MakeLTRB(contourBounds.left()*filterScale,contourBounds.top()*filterScale,contourBounds.right()*filterScale,contourBounds.bottom()*filterScale);
        context->clipRect(contourBounds);
        context->saveLayer(SkCanvas::SaveLayerRec(&contourBounds,&paint,SkCanvas::kInitWithPrevious_SaveLayerFlag));
        context->restore();context->restore();
    }

    // Native regular glass casts one inverse-clipped elevation field. Sample
    // the backdrop first so this layer never refracts its own shadow. Clear
    // glass has no default shadow; an explicit app shadow retains precedence.
    const auto material = Internal::BackdropShader::uniformValues(*_effect, "referenceAppearance");
    const auto appearance = Internal::BackdropShader::uniformValues(*_effect, "backdropLuma");
    const auto body = Internal::BackdropShader::uniformValues(*_effect, "bodyMode");
    if (!material.empty() && material.front()>0.5f && !appearance.empty() && !body.empty() && body.front()<0.5f
        && !shadowColor() && shadowOpacity()>0) {
        static thread_local const auto runtime=[] {
            auto source=std::string("uniform float2 shadowSize; uniform float cornerRadius, cornerCurve, shadowSigma, shadowOpacity; uniform float2 shadowOffset;\n")
                +Internal::layerCornerShaderUniforms+Internal::glassShadowGeometrySource()+Internal::glassShadowCDF
                +"half4 main(float2 xy) { float2 halfDim=shadowSize*0.5; float d=boxRoundedSDF(xy-shadowOffset-halfDim,halfDim,min(cornerRadius,min(halfDim.x,halfDim.y))); return half4(0,0,0,half(shadowOpacity*glassShadowTail(d/shadowSigma))); }";
            auto compiled=SkRuntimeEffect::MakeForShader(SkString(source));
            if(!compiled.effect) throw std::runtime_error("Glass shadow shader: "+std::string(compiled.errorText.c_str()));
            return compiled.effect;
        }();
        const auto parameters=Internal::glassShadow(layerBounds.size,appearance.front()<0.5f);
        SkRuntimeEffectBuilder builder(runtime);
        const float size[]={layerBounds.width(),layerBounds.height()},offset[]={0,parameters.offset};
        builder.uniform("shadowSize").set(size,2); builder.uniform("shadowOffset").set(offset,2);
        builder.uniform("cornerRadius")=cornerRadius();
        builder.uniform("cornerCurve")=cornerCurve()==CALayerCornerCurve::continuous?1.0f:0.0f;
        Internal::bindLayerCornerGeometry(builder, *runtime, layerBounds.size, cornerRadius());
        builder.uniform("shadowSigma")=parameters.sigma;
        builder.uniform("shadowOpacity")=parameters.opacity*shadowOpacity();
        SkPaint paint; paint.setShader(builder.makeShader());
        auto shadowBounds=rect;shadowBounds.outset(4*parameters.sigma+parameters.offset,4*parameters.sigma+parameters.offset);
        context->save();context->clipPath(shape,SkClipOp::kDifference,true);
        context->drawRect(shadowBounds,paint);context->restore();
    }
}
