#pragma once

#include <Geometry.h>
#include <UIColor.h>

#include <memory>
#include <optional>
#include <utility>

namespace NXKit {

class BackdropEffect;
class BackdropEffectView;
class UITraitCollection;

enum class UIGlassEffectStyle {
    regular,
    clear,
};

/**
 A UIKit-shaped description of NXKit's liquid-glass material.

 Style values provide defaults rather than locking the material. Every optical
 property below is an optional override: `std::nullopt` keeps the selected
 style's behavior, including the size and appearance adaptation of `regular`.
 Setting a value fixes only that property while the remaining properties keep
 following the style.

 `clear` is deliberately size-invariant and highly translucent. `regular`
 changes frost, vibrancy and neutral body tone with the hosting view's
 shortest dimension. Appearance follows inherited light/dark traits
 unless the NXKit backdrop-appearance option is explicitly enabled.

 The material adapts liquid_glass_widgets' lens to NXKit's live backdrop,
 with lighting and tint calibrated against native UIKit gallery captures.
 Interactive effects add touch-following sheen, size-dependent spring inflation and centered stretch without altering the view's public geometry.
 Long neutral regular platters use a color-preserving pressed gain rather than
 the stronger white press veil of short buttons. This response is calibrated
 against native tab bars in both themes; it is an approximation of UIKit.
 Refraction and body frost can be tuned independently, and the perimeter
 uses localized directional light; clear retains its sub-point hairline.
 `BackdropEffect::glass()` resolves the same clear material.
 */
class UIGlassEffect {
public:
    using Style = UIGlassEffectStyle;

    explicit UIGlassEffect(Style style = Style::regular);

    [[nodiscard]] Style style() const { return _style; }

    /** NXKit extension: apply color-dependent foreground vibrancy to this
     material's subviews. Opt-in; literal RGB content stays source-over by
     default, as in native glass. Intended for semantic/template foregrounds,
     not photographs or another glass surface. Reinstall the copied effect
     after changing this value. */
    [[nodiscard]] bool isContentVibrancyEnabled() const { return _contentVibrancy; }
    void setContentVibrancyEnabled(bool value) { _contentVibrancy = value; }

    /** Opt-in touch feedback, as in UIKit. Defaults to false. Reinstall the
     effect on its hosting view after changing this value (effects are copied). */
    [[nodiscard]] bool isInteractive() const { return _isInteractive; }
    void setInteractive(bool value) { _isInteractive = value; }

    /** NXKit extension restoring iOS 26-style contrast-driven appearance for
     small regular glass. Defaults to false. Large regular glass and clear
     glass keep the inherited appearance. The host also updates its ordinary
     subviews' color traits after the asynchronous backdrop sample completes.
     Reinstall the effect after changing this copied value. */
    [[nodiscard]] bool adaptsAppearanceToBackdrop() const { return _adaptsAppearanceToBackdrop; }
    void setAdaptsAppearanceToBackdrop(bool value) { _adaptsAppearanceToBackdrop = value; }

    [[nodiscard]] std::optional<NXFloat> maximumSampleRadius() const { return _maximumSampleRadius; }
    void setMaximumSampleRadius(std::optional<NXFloat> value);

    /** Figma's "Frost" control: the background-blur sigma in view-local points. */
    [[nodiscard]] std::optional<NXFloat> frostRadius() const { return _frostRadius; }
    void setFrostRadius(std::optional<NXFloat> value);

    /** Strength of the optical displacement. */
    [[nodiscard]] std::optional<NXFloat> refraction() const { return _refraction; }
    void setRefraction(std::optional<NXFloat> value);

    /** Normalized bevel depth. The native-calibrated 0.9 default gives a
     18-point curved edge, constrained by the hosting shape's corner radius. */
    [[nodiscard]] std::optional<NXFloat> refractionDepth() const { return _refractionDepth; }
    void setRefractionDepth(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> dispersion() const { return _dispersion; }
    void setDispersion(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> saturation() const { return _saturation; }
    void setSaturation(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> contrast() const { return _contrast; }
    void setContrast(std::optional<NXFloat> value);

    /** Additive post-contrast luminosity adjustment in the range -1...1. */
    [[nodiscard]] std::optional<NXFloat> luminosity() const { return _luminosity; }
    void setLuminosity(std::optional<NXFloat> value);

    /**
     Optional custom tint hue. When unset, clear uses white; regular uses
     `UIColor::systemBackground` in light appearance and a white veil in dark
     appearance. A supplied color uses UIKit-like clear/regular vibrancy; its
     alpha controls the contribution. An explicit tintOpacity override selects
     NXKit's low-level optical tint path and multiplies the color alpha.
     */
    [[nodiscard]] std::optional<UIColor> tintColor() const { return _tintColor; }
    void setTintColor(std::optional<UIColor> value) { _tintColor = std::move(value); }

    [[nodiscard]] std::optional<NXFloat> tintOpacity() const { return _tintOpacity; }
    void setTintOpacity(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> edgeStrength() const { return _edgeStrength; }
    void setEdgeStrength(std::optional<NXFloat> value);

    /** Meniscus absorption at the curved perimeter, in the range 0...1. */
    [[nodiscard]] std::optional<NXFloat> edgeAbsorption() const { return _edgeAbsorption; }
    void setEdgeAbsorption(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> specularStrength() const { return _specularStrength; }
    void setSpecularStrength(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> darkBackdropSpecularStrength() const { return _darkBackdropSpecularStrength; }
    void setDarkBackdropSpecularStrength(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> brightBackdropSpecularStrength() const { return _brightBackdropSpecularStrength; }
    void setBrightBackdropSpecularStrength(std::optional<NXFloat> value);

    /** Specular light angle in degrees, using +x as zero and +y as 90 degrees. */
    [[nodiscard]] std::optional<NXFloat> lightAngle() const { return _lightAngle; }
    void setLightAngle(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> specularDirectionalPower() const { return _specularDirectionalPower; }
    void setSpecularDirectionalPower(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> specularWidth() const { return _specularWidth; }
    void setSpecularWidth(std::optional<NXFloat> value);

    /** Exterior reflection sampling controls for the clear material. Regular
     uses neutral directional light calibrated to the native rim. */
    [[nodiscard]] std::optional<NXFloat> reflectionSampleDistance() const { return _reflectionSampleDistance; }
    void setReflectionSampleDistance(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> reflectionSpread() const { return _reflectionSpread; }
    void setReflectionSpread(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> brightBackdropShade() const { return _brightBackdropShade; }
    void setBrightBackdropShade(std::optional<NXFloat> value);

private:
    friend class BackdropEffect;
    friend class BackdropEffectView;
    friend class CABackdropEffectLayer;
    friend class CAGlassContainerLayer;
    friend class GlassMaterialTestHarness;

    [[nodiscard]] BackdropEffect _backdropEffect(
        NXSize size,
        const std::shared_ptr<UITraitCollection>& traits
    ) const;

    Style _style;
    bool _contentVibrancy = false;
    bool _isInteractive = false;
    bool _adaptsAppearanceToBackdrop = false;
    std::optional<NXFloat> _maximumSampleRadius;
    std::optional<NXFloat> _frostRadius;
    std::optional<NXFloat> _refraction;
    std::optional<NXFloat> _refractionDepth;
    std::optional<NXFloat> _dispersion;
    std::optional<NXFloat> _saturation;
    std::optional<NXFloat> _contrast;
    std::optional<NXFloat> _luminosity;
    std::optional<UIColor> _tintColor;
    std::optional<NXFloat> _tintOpacity;
    std::optional<NXFloat> _edgeStrength;
    std::optional<NXFloat> _edgeAbsorption;
    std::optional<NXFloat> _specularStrength;
    std::optional<NXFloat> _darkBackdropSpecularStrength;
    std::optional<NXFloat> _brightBackdropSpecularStrength;
    std::optional<NXFloat> _lightAngle;
    std::optional<NXFloat> _specularDirectionalPower;
    std::optional<NXFloat> _specularWidth;
    std::optional<NXFloat> _reflectionSampleDistance;
    std::optional<NXFloat> _reflectionSpread;
    std::optional<NXFloat> _brightBackdropShade;
};

}
