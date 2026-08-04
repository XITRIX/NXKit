#pragma once

#include <BackdropEffect.h>
#include <CALayer.h>

#include "include/core/SkRefCnt.h"

class SkRuntimeEffect;

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

    BackdropEffect _effect;
    sk_sp<SkRuntimeEffect> _runtimeEffect;
};

}
