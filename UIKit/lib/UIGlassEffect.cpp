#include <UIGlassEffect.h>
#include "NativeGlassShader.hpp"
#include "NativeGlassLighting.hpp"

#include <BackdropEffect.h>
#include <UITraitCollection.h>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <string>

using namespace NXKit;

namespace {
NXFloat interpolate(NXFloat lower, NXFloat upper, NXFloat progress) {
    return lower + (upper - lower) * progress;
}


void validateFinite(
    const std::optional<NXFloat>& value,
    const char* property
) {
    if (value && !std::isfinite(*value)) {
        throw std::invalid_argument(
            std::string("UIGlassEffect ") + property + " must be finite"
        );
    }
}

void validateRange(
    const std::optional<NXFloat>& value,
    NXFloat minimum,
    NXFloat maximum,
    const char* property
) {
    validateFinite(value, property);
    if (value && (*value < minimum || *value > maximum)) {
        throw std::invalid_argument(
            std::string("UIGlassEffect ") + property + " is outside its valid range"
        );
    }
}

void validateMinimum(
    const std::optional<NXFloat>& value,
    NXFloat minimum,
    const char* property
) {
    validateFinite(value, property);
    if (value && *value < minimum) {
        throw std::invalid_argument(
            std::string("UIGlassEffect ") + property + " is below its minimum"
        );
    }
}

}

UIGlassEffect::UIGlassEffect(Style style): _style(style) {}

void UIGlassEffect::setMaximumSampleRadius(std::optional<NXFloat> value) {
    validateMinimum(value, 0, "maximum sample radius");
    _maximumSampleRadius = value;
}

void UIGlassEffect::setFrostRadius(std::optional<NXFloat> value) {
    validateMinimum(value, 0, "frost radius");
    _frostRadius = value;
}

void UIGlassEffect::setRefraction(std::optional<NXFloat> value) {
    validateRange(value, 0, 1, "refraction");
    _refraction = value;
}

void UIGlassEffect::setRefractionDepth(std::optional<NXFloat> value) {
    validateRange(value, 0, 1, "refraction depth");
    _refractionDepth = value;
}

void UIGlassEffect::setDispersion(std::optional<NXFloat> value) {
    validateRange(value, 0, 1, "dispersion");
    _dispersion = value;
}

void UIGlassEffect::setSaturation(std::optional<NXFloat> value) {
    validateMinimum(value, 0, "saturation");
    _saturation = value;
}

void UIGlassEffect::setContrast(std::optional<NXFloat> value) {
    validateMinimum(value, 0, "contrast");
    _contrast = value;
}

void UIGlassEffect::setLuminosity(std::optional<NXFloat> value) {
    validateRange(value, -1, 1, "luminosity");
    _luminosity = value;
}

void UIGlassEffect::setTintOpacity(std::optional<NXFloat> value) {
    validateRange(value, 0, 1, "tint opacity");
    _tintOpacity = value;
}

void UIGlassEffect::setEdgeStrength(std::optional<NXFloat> value) {
    validateRange(value, 0, 1, "edge strength");
    _edgeStrength = value;
}

void UIGlassEffect::setEdgeAbsorption(std::optional<NXFloat> value) {
    validateRange(value, 0, 1, "edge absorption");
    _edgeAbsorption = value;
}

void UIGlassEffect::setSpecularStrength(std::optional<NXFloat> value) {
    validateRange(value, 0, 1, "specular strength");
    _specularStrength = value;
}

void UIGlassEffect::setDarkBackdropSpecularStrength(std::optional<NXFloat> value) {
    validateRange(value, 0, 1, "dark-backdrop specular strength");
    _darkBackdropSpecularStrength = value;
}

void UIGlassEffect::setBrightBackdropSpecularStrength(std::optional<NXFloat> value) {
    validateRange(value, 0, 1, "bright-backdrop specular strength");
    _brightBackdropSpecularStrength = value;
}

void UIGlassEffect::setLightAngle(std::optional<NXFloat> value) {
    validateFinite(value, "light angle");
    _lightAngle = value;
}

void UIGlassEffect::setSpecularDirectionalPower(std::optional<NXFloat> value) {
    validateMinimum(value, 1, "specular directional power");
    _specularDirectionalPower = value;
}

void UIGlassEffect::setSpecularWidth(std::optional<NXFloat> value) {
    validateMinimum(value, 0, "specular width");
    _specularWidth = value;
}

void UIGlassEffect::setReflectionSampleDistance(std::optional<NXFloat> value) {
    validateMinimum(value, 0, "reflection sample distance");
    _reflectionSampleDistance = value;
}

void UIGlassEffect::setReflectionSpread(std::optional<NXFloat> value) {
    validateMinimum(value, 0, "reflection spread");
    _reflectionSpread = value;
}

void UIGlassEffect::setBrightBackdropShade(std::optional<NXFloat> value) {
    validateRange(value, 0, 0.1f, "bright-backdrop shade");
    _brightBackdropShade = value;
}

BackdropEffect UIGlassEffect::_backdropEffect(
    NXSize size,
    const std::shared_ptr<UITraitCollection>& traits
) const {
    const auto style = traits ? traits->userInterfaceStyle() : UIUserInterfaceStyle::unspecified;
    const auto amount = traits ? traits->glassTintAmount() : UITraitCollection::defaultGlassTintAmount;
    auto resolvedSize = size;
    if (size.width == 0 && size.height == 0) {
        const auto reference = Internal::nativeGlassProfile(_style, style, amount);
        resolvedSize = {reference.context.width, reference.context.height};
    }
    auto profile = Internal::nativeGlassProfile(_style, style, amount, resolvedSize,
        _tintColor.has_value() && !_tintOpacity.has_value());
    const auto traitScale = traits ? traits->displayScale() : 1.f;
    Internal::resolveNativeGlassDisplayScale(profile,
        std::isfinite(traitScale) && traitScale > 0 ? traitScale : 1.f);
    if (_frostRadius) {
        profile.blurRadius = *_frostRadius;
        profile.blurFillRadius = profile.bleedBlurRadius = profile.shadowBlurRadius = 0;
        profile.blurOpacity0 = profile.blurOpacity1 = profile.blurOpacity2 = profile.blurOpacity3 = 1;
        profile.blurFillNormalOpacity = profile.blurFillLightenOpacity = profile.blurFillDarkenOpacity = 0;
    }
    if (_refraction) {
        const auto gain = *_refraction / .55f;
        profile.innerRefractionAmount *= gain; profile.outerRefractionAmount *= gain;
    }
    if (_refractionDepth) {
        const auto height = *_refractionDepth * 20;
        profile.innerRefractionHeight = profile.outerRefractionHeight = height;
    }
    if (_saturation) profile.faceSaturation = *_saturation;
    if (_contrast) {
        profile.faceBlack = 0; profile.faceWhite = *_contrast;
        profile.faceColorMatrixMaxLuma = profile.faceColorMatrixMaxLumaSDR = 1;
        profile.faceColorMatrixFillColor = {0,0,0,0};
    }
    if (_luminosity) { profile.faceBlack += *_luminosity; profile.faceWhite += *_luminosity; }
    if (_specularStrength) {
        const auto strength = *_specularStrength / .52f;
        profile.highlight.keyAmount *= strength; profile.highlight.fillAmount *= strength;
        profile.keyFillHighlightAmount *= strength;
    }
    if (_edgeStrength) {
        profile.highlight.keyAmount *= *_edgeStrength; profile.highlight.fillAmount *= *_edgeStrength;
        profile.keyFillHighlightAmount *= *_edgeStrength;
        profile.bleedOpacity *= *_edgeStrength; profile.ringShadowOpacity *= *_edgeStrength;
    }
    if (_specularWidth) {
        const auto width = *_specularWidth / .8f;
        profile.highlight.keyHeight *= width; profile.highlight.fillHeight *= width;
        profile.keyFillHighlightHeight *= width;
    }
    if (_lightAngle) {
        const auto angle = *_lightAngle * std::numbers::pi_v<NXFloat> / 180;
        profile.highlight.keyAngle = angle + std::numbers::pi_v<NXFloat> / 2;
        profile.highlight.fillAngle = profile.highlight.keyAngle + std::numbers::pi_v<NXFloat>;
        profile.keyFillHighlightAngle = angle;
    }
    if (_specularDirectionalPower) {
        const auto spread = std::numbers::pi_v<NXFloat> / std::sqrt(std::max(*_specularDirectionalPower,1.f));
        profile.highlight.keySpread = profile.highlight.fillSpread = spread;
    }
    const auto tint = _tintColor.value_or(UIColor::white).resolvedColorWithTraitCollection(traits);
    auto effect = Internal::makeNativeGlassEffect(profile, _style, tint,
        _tintColor.has_value() && !_tintOpacity.has_value(), _maximumSampleRadius.value_or(
            std::max({std::abs(profile.innerRefractionAmount),std::abs(profile.outerRefractionAmount),
                (profile.bleedOpacity>0 && profile.bleedHeight>0)?std::abs(profile.bleedAmount):0,
                (profile.shadowOpacity>0 || profile.sdrShadowOpacity>0)?std::abs(profile.shadowAmount):0, _reflectionSampleDistance.value_or(0)+_reflectionSpread.value_or(0)})));
    if (_frostRadius) {
        // The public override is Skia sigma in view points, independent of the
        // private CAFilter radius conversion and native backdrop downsampling.
        effect.setBackdropBlurRadius(*_frostRadius);
        effect.setBackdropScale(1);
    }
    effect.setUniform("nativeDispersion", _dispersion.value_or(0));
    effect.setUniform("nativeEdgeAbsorption", _edgeAbsorption.value_or(0));
    effect.setUniform("nativeTintOpacity", _tintOpacity.value_or(-1));
    effect.setUniform("nativeBrightBackdropShade", _brightBackdropShade.value_or(0));
    // Portable reflection controls retain separate roles without replacing the
    // native pass data. Their defaults are identity adjustments.
    effect.setUniform("nativeDarkSpecularGain", _darkBackdropSpecularStrength.value_or(.52f) / .52f);
    effect.setUniform("nativeBrightSpecularGain", _brightBackdropSpecularStrength.value_or(.52f) / .52f);
    effect.setUniform("nativeReflectionDistance", _reflectionSampleDistance.value_or(0));
    effect.setUniform("nativeReflectionSpread", _reflectionSpread.value_or(0));
    return effect;
}
