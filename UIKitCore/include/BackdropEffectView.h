#pragma once

#include <BackdropEffect.h>
#include <UIGlassEffect.h>
#include <UIView.h>

namespace NXKit {

enum class UITouchPhase;

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
    ~BackdropEffectView() override;

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
    bool applyXMLAttribute(const std::string& name, const std::string& value) override;

private:
    friend class GlassInteractionTestHarness;
    std::shared_ptr<UITraitCollection> traitsForSubviews() const override;
    void _configureAppearance(bool propagateTraits = true);
    void _updateAppearance(std::optional<NXFloat> luminance, bool propagateTraits = true);
    std::shared_ptr<UITraitCollection> _contentTraits;
    std::optional<NXFloat> _backdropLuminance;
    struct InteractionState;
    void _configureInteraction();
    void _handleGlassTouch(UITouchPhase phase, const std::shared_ptr<UITouch>& touch);
    void _advanceInteraction(double seconds);
    void _wakeInteraction();
    void _resetInteraction();
    std::unique_ptr<InteractionState> _interaction;

    std::shared_ptr<CABackdropEffectLayer> _backdropEffectLayer() const;
    void _updateGlassEffect(bool force = false);

    std::optional<UIGlassEffect> _glassEffect;
    NXSize _resolvedGlassSize;
    UIUserInterfaceStyle _resolvedInterfaceStyle = UIUserInterfaceStyle::unspecified;
    bool _hasResolvedGlass = false;
};

}
