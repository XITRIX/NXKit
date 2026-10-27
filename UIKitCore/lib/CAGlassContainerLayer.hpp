#pragma once
#include <CALayer.h>
#include <UIGlassContainerEffect.h>
#include "include/effects/SkRuntimeEffect.h"

namespace NXKit {
class CABackdropEffectLayer;
class CAGlassContainerLayer final : public CALayer {
public:
    explicit CAGlassContainerLayer(const UIGlassContainerEffect& effect);
    explicit CAGlassContainerLayer(CAGlassContainerLayer* layer);
    std::shared_ptr<CALayer> copy() override;
    void setEffect(const UIGlassContainerEffect& effect);
    UIGlassContainerEffect effect() const { return _effect; }
    void draw(SkCanvas* canvas) override;
    static bool contains(const CABackdropEffectLayer* layer);
protected:
    void renderSublayers(SkCanvas* canvas) override;
private:
    struct Item;
    void collect(const std::shared_ptr<CALayer>& layer, const SkMatrix& parent, float alpha,
                 std::vector<Item>& items);
    UIGlassContainerEffect _effect;
    std::vector<const CABackdropEffectLayer*> _renderedItems;
    sk_sp<SkRuntimeEffect> _runtime;
    size_t _runtimeCount = 0;
};
}
