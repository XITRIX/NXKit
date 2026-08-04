#pragma once

#include <BackdropEffect.h>
#include <CALayer.h>

#include "include/core/SkRefCnt.h"

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

private:
    static sk_sp<SkRuntimeEffect> compile(const BackdropEffect& effect);
    static void validateUniforms(const BackdropEffect& effect, const SkRuntimeEffect& runtimeEffect);
    sk_sp<SkImageFilter> imageFilterForCurrentState();
    void invalidateImageFilterCache();

    BackdropEffect _effect;
    sk_sp<SkRuntimeEffect> _runtimeEffect;
    sk_sp<SkImageFilter> _cachedImageFilter;
    NXSize _cachedFilterSize;
    NXFloat _cachedFilterCornerRadius = 0;
    NXFloat _cachedFilterContentsScale = 0;
    bool _hasCachedFilterState = false;
};

}
