#include <UIGlassEffect.h>

#include <BackdropEffect.h>
#include <UITraitCollection.h>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <string>

using namespace NXKit;

namespace {

struct ResolvedGlassParameters {
    NXFloat maximumSampleRadius;
    NXFloat frostRadius;
    NXFloat refraction;
    NXFloat refractionDepth;
    NXFloat dispersion;
    NXFloat saturation;
    NXFloat contrast;
    NXFloat luminosity;
    UIColor tintColor;
    NXFloat tintOpacity;
    NXFloat edgeStrength;
    NXFloat specularStrength;
    NXFloat darkBackdropSpecularStrength;
    NXFloat brightBackdropSpecularStrength;
    NXFloat lightAngle;
    NXFloat specularDirectionalPower;
    NXFloat specularWidth;
    NXFloat reflectionSampleDistance;
    NXFloat reflectionSpread;
    NXFloat brightBackdropShade;
};

NXFloat smoothstep(NXFloat lower, NXFloat upper, NXFloat value) {
    const auto progress = std::clamp((value - lower) / (upper - lower), 0.0f, 1.0f);
    return progress * progress * (3.0f - 2.0f * progress);
}

NXFloat interpolate(NXFloat lower, NXFloat upper, NXFloat progress) {
    return lower + (upper - lower) * progress;
}

ResolvedGlassParameters styleDefaults(
    UIGlassEffect::Style style,
    NXSize size,
    UIUserInterfaceStyle interfaceStyle
) {
    if (style == UIGlassEffect::Style::clear) {
        return {
            .maximumSampleRadius = 64.0f,
            .frostRadius = 2.5f,
            .refraction = 0.55f,
            .refractionDepth = 0.90f,
            .dispersion = 0.32f,
            .saturation = 1.12f,
            .contrast = 1.04f,
            .luminosity = 0.0f,
            .tintColor = UIColor::white,
            .tintOpacity = 32.0f / 255.0f,
            .edgeStrength = 1.0f,
            .specularStrength = 0.52f,
            .darkBackdropSpecularStrength = 0.50f,
            .brightBackdropSpecularStrength = 0.14f,
            .lightAngle = -90.0f,
            .specularDirectionalPower = 3.0f,
            .specularWidth = 4.0f,
            .reflectionSampleDistance = 12.0f,
            .reflectionSpread = 3.0f,
            .brightBackdropShade = 0.008f,
        };
    }

    // Apple's regular material adapts with size. The supplied iOS captures
    // show the small lens close to clear glass and the large lens with almost
    // all high-frequency backdrop detail removed. Use the shortest dimension
    // so controls stay lightweight while alerts, menus, and sidebars become
    // progressively more prominent. The transition is smooth to avoid visible
    // steps during ordinary layout changes.
    const auto shortestDimension = std::max<NXFloat>(
        0,
        std::min(size.width, size.height)
    );
    const auto sizeAmount = smoothstep(64.0f, 220.0f, shortestDimension);
    const auto maximumTintOpacity = interfaceStyle == UIUserInterfaceStyle::dark
        ? 0.36f
        : 0.28f;
    const auto maximumLuminosity = interfaceStyle == UIUserInterfaceStyle::dark
        ? 0.0f
        : 0.12f;
    return {
        // Keep the declared backdrop extent at the proven cross-platform
        // limit used by the existing glass renderer. The largest regular
        // frost kernel still fits comfortably inside it.
        .maximumSampleRadius = 64.0f,
        .frostRadius = interpolate(3.0f, 24.0f, sizeAmount),
        .refraction = interpolate(0.46f, 0.38f, sizeAmount),
        .refractionDepth = interpolate(0.82f, 0.72f, sizeAmount),
        .dispersion = interpolate(0.20f, 0.12f, sizeAmount),
        .saturation = interpolate(1.06f, 0.96f, sizeAmount),
        .contrast = interpolate(1.02f, 0.88f, sizeAmount),
        .luminosity = interpolate(0.0f, maximumLuminosity, sizeAmount),
        .tintColor = UIColor::systemBackground,
        .tintOpacity = interpolate(0.06f, maximumTintOpacity, sizeAmount),
        .edgeStrength = 1.0f,
        .specularStrength = interpolate(0.44f, 0.34f, sizeAmount),
        .darkBackdropSpecularStrength = 0.42f,
        .brightBackdropSpecularStrength = 0.12f,
        .lightAngle = -90.0f,
        .specularDirectionalPower = 3.0f,
        .specularWidth = interpolate(3.5f, 5.0f, sizeAmount),
        .reflectionSampleDistance = 12.0f,
        .reflectionSpread = 3.0f,
        .brightBackdropShade = 0.008f,
    };
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
    const auto interfaceStyle = traits
        ? traits->userInterfaceStyle()
        : UIUserInterfaceStyle::unspecified;
    auto parameters = styleDefaults(_style, size, interfaceStyle);

    parameters.maximumSampleRadius = _maximumSampleRadius.value_or(
        parameters.maximumSampleRadius
    );
    parameters.frostRadius = _frostRadius.value_or(parameters.frostRadius);
    parameters.refraction = _refraction.value_or(parameters.refraction);
    parameters.refractionDepth = _refractionDepth.value_or(parameters.refractionDepth);
    parameters.dispersion = _dispersion.value_or(parameters.dispersion);
    parameters.saturation = _saturation.value_or(parameters.saturation);
    parameters.contrast = _contrast.value_or(parameters.contrast);
    parameters.luminosity = _luminosity.value_or(parameters.luminosity);
    parameters.tintOpacity = _tintOpacity.value_or(parameters.tintOpacity);
    parameters.edgeStrength = _edgeStrength.value_or(parameters.edgeStrength);
    parameters.specularStrength = _specularStrength.value_or(parameters.specularStrength);
    parameters.darkBackdropSpecularStrength = _darkBackdropSpecularStrength.value_or(
        parameters.darkBackdropSpecularStrength
    );
    parameters.brightBackdropSpecularStrength = _brightBackdropSpecularStrength.value_or(
        parameters.brightBackdropSpecularStrength
    );
    parameters.lightAngle = _lightAngle.value_or(parameters.lightAngle);
    parameters.specularDirectionalPower = _specularDirectionalPower.value_or(
        parameters.specularDirectionalPower
    );
    parameters.specularWidth = _specularWidth.value_or(parameters.specularWidth);
    parameters.reflectionSampleDistance = _reflectionSampleDistance.value_or(
        parameters.reflectionSampleDistance
    );
    parameters.reflectionSpread = _reflectionSpread.value_or(parameters.reflectionSpread);
    parameters.brightBackdropShade = _brightBackdropShade.value_or(
        parameters.brightBackdropShade
    );

    const auto tint = _tintColor.value_or(parameters.tintColor);
    const auto tintRaw = tint.resolvedRaw(traits);
    const auto tintAlpha = static_cast<NXFloat>((tintRaw >> 24) & 0xff) / 255.0f;
    const auto resolvedTint = UIColor(
        static_cast<unsigned char>((tintRaw >> 16) & 0xff),
        static_cast<unsigned char>((tintRaw >> 8) & 0xff),
        static_cast<unsigned char>(tintRaw & 0xff),
        static_cast<unsigned char>(
            std::round(255.0f * parameters.tintOpacity * tintAlpha)
        )
    );

    auto effect = BackdropEffect::glassShaderEffect(
        parameters.maximumSampleRadius,
        parameters.frostRadius
    );
    // The original shader calls the curved-edge depth `refraction` and the
    // displacement strength `curve`. Keep those private uniform spellings
    // while exposing the Figma/optics terminology through UIGlassEffect.
    effect.setUniform("refraction", parameters.refractionDepth);
    effect.setUniform("curve", parameters.refraction);
    effect.setUniform("dispersion", parameters.dispersion);
    effect.setUniform("saturation", parameters.saturation);
    effect.setUniform("contrast", parameters.contrast);
    effect.setUniform("luminosity", parameters.luminosity);
    effect.setUniform("tint", resolvedTint);
    effect.setUniform("edge", parameters.edgeStrength);
    effect.setUniform("specStrength", parameters.specularStrength);
    effect.setUniform(
        "specFallbackStrength",
        parameters.darkBackdropSpecularStrength
    );
    effect.setUniform(
        "specBrightFallbackStrength",
        parameters.brightBackdropSpecularStrength
    );

    const auto angle = parameters.lightAngle
        * std::numbers::pi_v<NXFloat>
        / 180.0f;
    effect.setUniform("specLightDirection", NXPoint(std::cos(angle), std::sin(angle)));
    effect.setUniform("specDirectionalPower", parameters.specularDirectionalPower);
    effect.setUniform("specWidthPx", parameters.specularWidth);
    effect.setUniform("reflectionSamplePx", parameters.reflectionSampleDistance);
    effect.setUniform("reflectionSpreadPx", parameters.reflectionSpread);
    effect.setUniform("brightBackdropShade", parameters.brightBackdropShade);
    return effect;
}
