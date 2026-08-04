#pragma once

#include <BackdropEffect.h>
#include <UIView.h>

namespace NXKit {

/**
 A view that applies an SkSL effect to the already-rendered content behind it.

 Its subviews are drawn normally above the affected backdrop. The default
 initializer installs BackdropEffect::glass().
 */
class BackdropEffectView: public UIView {
public:
    static std::shared_ptr<UIView> init() { return new_shared<BackdropEffectView>(); }

    BackdropEffectView();
    explicit BackdropEffectView(const BackdropEffect& effect);

    /**
     Compiles and installs effect. Invalid SkSL, child bindings, or uniform
     values throw std::invalid_argument and leave the previous effect intact.
     */
    void setEffect(const BackdropEffect& effect);
    [[nodiscard]] BackdropEffect effect() const;

private:
    std::shared_ptr<CABackdropEffectLayer> _backdropEffectLayer() const;
};

}
