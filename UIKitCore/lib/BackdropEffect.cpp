#include <BackdropEffect.h>

#include <cmath>
#include <stdexcept>
#include <utility>

using namespace NXKit;

namespace {
bool isAutomaticUniformName(const std::string& name) {
    return name == "resolution"
        || name == "lensCenter"
        || name == "lensSize"
        || name == "cornerRadius"
        || name == "cornerCurve"
        || name == "cornerTop" || name == "cornerTopCircle"
        || name == "cornerRight" || name == "cornerRightCircle"
        || name == "externalShapeCoverage"
        || name == "contentScale"
        || name == "filterScale"
        || name == "maximumSampleRadius"
        || name == "interactionIntensity"
        || name == "interactionLocation";
}

void validateName(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("BackdropEffect uniform name cannot be empty");
    }
    if (isAutomaticUniformName(name)) {
        throw std::invalid_argument(
            "BackdropEffect uniform '" + name + "' is populated automatically"
        );
    }
}

}

BackdropEffect::BackdropEffect(
    std::string shaderSource,
    std::string backdropShaderName,
    NXFloat maximumSampleRadius,
    NXFloat backdropBlurRadius
):
    _shaderSource(std::move(shaderSource)),
    _backdropShaderName(std::move(backdropShaderName)),
    _maximumSampleRadius(maximumSampleRadius),
    _backdropBlurRadius(backdropBlurRadius)
{
    if (_shaderSource.empty()) {
        throw std::invalid_argument("BackdropEffect shader source cannot be empty");
    }
    if (_backdropShaderName.empty()) {
        throw std::invalid_argument("BackdropEffect backdrop shader name cannot be empty");
    }
    if (!std::isfinite(_maximumSampleRadius) || _maximumSampleRadius < 0) {
        throw std::invalid_argument(
            "BackdropEffect maximum sample radius must be finite and non-negative"
        );
    }
    if (!std::isfinite(_backdropBlurRadius) || _backdropBlurRadius < 0) {
        throw std::invalid_argument(
            "BackdropEffect blur radius must be finite and non-negative"
        );
    }
}

std::optional<std::vector<NXFloat>> BackdropEffect::uniform(
    const std::string& name
) const {
    const auto iterator = _uniforms.find(name);
    if (iterator == _uniforms.end()) return std::nullopt;
    return iterator->second;
}

bool BackdropEffect::isAutomaticUniform(const std::string& name) {
    return isAutomaticUniformName(name);
}

void BackdropEffect::setFloatUniform(const std::string& name, std::vector<NXFloat> value) {
    validateName(name);
    if (value.empty()) {
        throw std::invalid_argument("BackdropEffect uniform values cannot be empty");
    }
    for (const auto component : value) {
        if (!std::isfinite(component)) {
            throw std::invalid_argument("BackdropEffect uniform values must be finite");
        }
    }
    _uniforms[name] = std::move(value);
}

void BackdropEffect::setUniform(const std::string& name, NXFloat value) {
    setFloatUniform(name, {value});
}

void BackdropEffect::setUniform(const std::string& name, NXPoint value) {
    setFloatUniform(name, {value.x, value.y});
}

void BackdropEffect::setUniform(const std::string& name, NXSize value) {
    setFloatUniform(name, {value.width, value.height});
}

void BackdropEffect::setUniform(const std::string& name, const std::array<NXFloat, 3>& value) {
    setFloatUniform(name, std::vector<NXFloat>(value.begin(), value.end()));
}

void BackdropEffect::setUniform(const std::string& name, const std::array<NXFloat, 4>& value) {
    setFloatUniform(name, std::vector<NXFloat>(value.begin(), value.end()));
}

void BackdropEffect::setUniform(const std::string& name, const std::vector<NXFloat>& value) {
    setFloatUniform(name, value);
}

void BackdropEffect::setUniform(const std::string& name, const UIColor& value) {
    setFloatUniform(name, {
        static_cast<NXFloat>(value.r()) / 255.0f,
        static_cast<NXFloat>(value.g()) / 255.0f,
        static_cast<NXFloat>(value.b()) / 255.0f,
        static_cast<NXFloat>(value.a()) / 255.0f
    });
}

void BackdropEffect::removeUniform(const std::string& name) {
    if (isAutomaticUniform(name)) return;
    _uniforms.erase(name);
}
