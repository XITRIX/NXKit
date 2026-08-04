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
 increases its frost radius and system-background tint as the hosting view's
 shortest dimension grows. In light mode it also lifts the material's
 luminosity, while dark mode resolves the tint without that lift.
 */
class UIGlassEffect {
public:
    using Style = UIGlassEffectStyle;

    explicit UIGlassEffect(Style style = Style::regular);

    [[nodiscard]] Style style() const { return _style; }

    [[nodiscard]] std::optional<NXFloat> maximumSampleRadius() const { return _maximumSampleRadius; }
    void setMaximumSampleRadius(std::optional<NXFloat> value);

    /** Figma's "Frost" control: the background-blur sigma in view-local points. */
    [[nodiscard]] std::optional<NXFloat> frostRadius() const { return _frostRadius; }
    void setFrostRadius(std::optional<NXFloat> value);

    /** Strength of the optical displacement. */
    [[nodiscard]] std::optional<NXFloat> refraction() const { return _refraction; }
    void setRefraction(std::optional<NXFloat> value);

    /** Fraction of the shortest half-dimension occupied by the curved lens edge. */
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
     Optional custom tint hue. When unset, clear uses white and regular uses
     `UIColor::systemBackground`. The color's alpha multiplies tintOpacity().
     */
    [[nodiscard]] std::optional<UIColor> tintColor() const { return _tintColor; }
    void setTintColor(std::optional<UIColor> value) { _tintColor = std::move(value); }

    [[nodiscard]] std::optional<NXFloat> tintOpacity() const { return _tintOpacity; }
    void setTintOpacity(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> edgeStrength() const { return _edgeStrength; }
    void setEdgeStrength(std::optional<NXFloat> value);

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

    [[nodiscard]] std::optional<NXFloat> reflectionSampleDistance() const { return _reflectionSampleDistance; }
    void setReflectionSampleDistance(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> reflectionSpread() const { return _reflectionSpread; }
    void setReflectionSpread(std::optional<NXFloat> value);

    [[nodiscard]] std::optional<NXFloat> brightBackdropShade() const { return _brightBackdropShade; }
    void setBrightBackdropShade(std::optional<NXFloat> value);

private:
    friend class BackdropEffect;
    friend class BackdropEffectView;

    [[nodiscard]] BackdropEffect _backdropEffect(
        NXSize size,
        const std::shared_ptr<UITraitCollection>& traits
    ) const;

    Style _style;
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
