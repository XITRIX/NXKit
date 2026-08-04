#pragma once

#include <BackdropEffect.h>
#include <UIGlassEffect.h>
#include <UIView.h>

namespace NXKit {

/**
 A view that applies an SkSL effect to the already-rendered content behind it.

 Its subviews are drawn normally above the affected backdrop. The default
 initializer installs a regular UIGlassEffect.
 */
class BackdropEffectView: public UIView {
public:
    static std::shared_ptr<UIView> init() { return new_shared<BackdropEffectView>(); }

    BackdropEffectView();
    explicit BackdropEffectView(const BackdropEffect& effect);
    explicit BackdropEffectView(const UIGlassEffect& effect);

    void setFrame(NXRect frame) override;
    void setBounds(NXRect bounds) override;
    void traitCollectionDidChange(
        std::shared_ptr<UITraitCollection> previousTraitCollection
    ) override;

    /**
     Compiles and installs effect. Invalid SkSL, child bindings, or uniform
     values throw std::invalid_argument and leave the previous effect intact.
     */
    void setEffect(const BackdropEffect& effect);
    void setEffect(const UIGlassEffect& effect);
    [[nodiscard]] BackdropEffect effect() const;
    [[nodiscard]] std::optional<UIGlassEffect> glassEffect() const { return _glassEffect; }

private:
    std::shared_ptr<CABackdropEffectLayer> _backdropEffectLayer() const;
    void _updateGlassEffect(bool force = false);

    std::optional<UIGlassEffect> _glassEffect;
    NXSize _resolvedGlassSize;
    UIUserInterfaceStyle _resolvedInterfaceStyle = UIUserInterfaceStyle::unspecified;
    bool _hasResolvedGlass = false;
};

}
