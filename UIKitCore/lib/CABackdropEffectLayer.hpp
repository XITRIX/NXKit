#pragma once

#include <BackdropEffect.h>
#include <CALayer.h>
#include <UIGlassEffect.h>
#include "GlassInteraction.hpp"

#include "include/core/SkRefCnt.h"
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
    [[nodiscard]] BackdropEffect effect() const { return _effect; }

    void setGlassInteraction(const Internal::GlassInteraction& interaction);

protected:
    NXAffineTransform renderingTransform() const override;

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
    sk_sp<SkImageFilter> imageFilterForCurrentState();
    void invalidateImageFilterCache();

    BackdropEffect _effect;
    sk_sp<SkRuntimeEffect> _runtimeEffect;
    sk_sp<SkImageFilter> _cachedImageFilter;
    NXSize _cachedFilterSize;
    NXFloat _cachedFilterCornerRadius = 0;
    CALayerCornerCurve _cachedFilterCornerCurve = CALayerCornerCurve::continuous;
    NXFloat _cachedFilterContentsScale = 0;
    bool _hasCachedFilterState = false;
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
