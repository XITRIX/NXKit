#pragma once

#include <BackdropEffect.h>
#include <CALayer.h>
#include <UIGlassEffect.h>
#include <private/UIBlurEffect.h>
#include "GlassInteraction.hpp"

#include "include/core/SkRefCnt.h"
#include <array>
#include <functional>
#include <optional>

class SkRuntimeEffect;
class SkImageFilter;

namespace NXKit {

class CABackdropEffectLayer final: public CALayer {
public:
    explicit CABackdropEffectLayer(const BackdropEffect& effect);
    explicit CABackdropEffectLayer(CABackdropEffectLayer* layer);
    ~CABackdropEffectLayer() override = default;

    void draw(SkCanvas* context) override;
    std::shared_ptr<CALayer> copy() override;

    void setEffect(const BackdropEffect& effect);
    void setEffect(const UIBlurEffect& effect);
    [[nodiscard]] BackdropEffect effect() const { return *_effect; }
    [[nodiscard]] bool samplesOnlyLocalBackdrop() const {
        return !_variableBlur&&!_glassMaterial&&_effect&&_effect->maximumSampleRadius()==0
            &&_effect->backdropBlurRadius()==0&&_effect->backdropScale()==1
            &&!_effect->backdropPyramid()&&_effect->backdropInputs().empty()&&_effect->outputOutset()==0;
    }

    void setGlassInteraction(const Internal::GlassInteraction& interaction);

protected:
    NXAffineTransform renderingTransform() const override;
    NXFloat renderingOutset() const override { return _effect ? _effect->outputOutset() : 0; }
    void renderSublayers(SkCanvas* canvas) override;

private:
    friend class GlassInteractionTestHarness;
    friend class BackdropEffectView;
    friend class CAGlassContainerLayer;
    struct AppearanceObservation;
    void setAppearanceObserver(std::function<void(std::optional<NXFloat>, NXSize)> observer);
    void setGlassMaterial(std::optional<UIGlassEffect> glass, std::shared_ptr<UITraitCollection> traits);
    void observeBackdrop(SkCanvas* context);
    static sk_sp<SkRuntimeEffect> compile(const BackdropEffect& effect);
    static void validateUniforms(const BackdropEffect& effect, const SkRuntimeEffect& runtimeEffect);
    sk_sp<SkImageFilter> imageFilterForCurrentState(SkCanvas* canvas=nullptr);
    std::shared_ptr<const BackdropEffect> resolvedEffect(NXSize size);
    void invalidateImageFilterCache();
    struct ResolvedMaterial {
        NXSize size;
        BackdropEffect effect;
    };
    // Immutable descriptions are shared by model/presentation copies.
    // A grouped member can resolve both its own size and the group's shared
    // material size. Keep both so steady grouping does not churn the cache.
    std::array<std::shared_ptr<const ResolvedMaterial>, 2> _resolvedMaterials;

    std::shared_ptr<const BackdropEffect> _effect;
    std::optional<UIBlurEffect> _variableBlur;
    sk_sp<SkRuntimeEffect> _runtimeEffect;
    sk_sp<SkImageFilter> _cachedImageFilter;
    NXSize _cachedFilterSize;
    NXFloat _cachedFilterCornerRadius = 0;
    CALayerCornerCurve _cachedFilterCornerCurve = CALayerCornerCurve::continuous;
    NXFloat _cachedFilterContentsScale = 0;
    bool _hasCachedFilterState = false;
    NXFloat _cachedGlassShadowOpacity = 1;
    bool _cachedGlassHasExplicitShadow = false;
    NXFloat _cachedInteractionGlow = 0;
    NXPoint _cachedInteractionLocation;
    // Shared by model and presentation copies, so concurrent app animations
    // continue to receive the live touch feedback.
    std::shared_ptr<Internal::GlassInteraction> _interaction;
    std::shared_ptr<AppearanceObservation> _appearanceObservation;
    std::optional<UIGlassEffect> _glassMaterial;
    std::shared_ptr<UITraitCollection> _glassTraits;
};

}
