#include <BackdropEffect.h>

#include <cmath>
#include <algorithm>
#include <cctype>
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
        || name == "backdropPyramidRect" || name == "backdropPyramidInfo"
        || name == "maximumSampleRadius"
        || name == "interactionIntensity"
        || name == "interactionLocation";
}

BackdropBlurKernel validatedKernel(BackdropBlurKernel k) {
    if (!std::isfinite(k.sigma0) || !std::isfinite(k.sigma1)
        || k.sigma0 < 0 || k.sigma1 < 0 || !std::isfinite(k.weight) || k.weight < 0 || k.weight > 1)
        throw std::invalid_argument("Backdrop kernel requires finite nonnegative sigmas and weight in [0, 1]");
    if (k.weight == 1) return {k.sigma1,k.sigma1,0};
    if (k.weight == 0 || k.sigma0 == k.sigma1) return {k.sigma0,k.sigma0,0};
    return k;
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
    _backdropBlurRadius(backdropBlurRadius),
    _backdropBlurKernel{backdropBlurRadius,backdropBlurRadius,0}
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

void BackdropEffect::setBackdropInput(std::string name, NXFloat radius) {
    setBackdropInput(std::move(name),BackdropBlurKernel{radius,radius,0});
}

void BackdropEffect::setBackdropInput(std::string name, BackdropBlurKernel kernel) {
    kernel = validatedKernel(kernel);
    const auto identifierStart = [](unsigned char c) { return std::isalpha(c) || c == '_'; };
    if (name.empty() || !identifierStart(name.front())
        || !std::all_of(name.begin(), name.end(), [](unsigned char c) { return std::isalnum(c) || c == '_'; }))
        throw std::invalid_argument("Backdrop input must be a shader identifier");
    if (name == _backdropShaderName || name == "sharpContent")
        throw std::invalid_argument("Primary and sharp backdrop inputs are reserved");
    _backdropInputs[name] = std::max(kernel.sigma0,kernel.sigma1);
    _backdropInputKernels[std::move(name)] = kernel;
}

void BackdropEffect::setBackdropBlurRadius(NXFloat value) {
    if (!std::isfinite(value) || value < 0)
        throw std::invalid_argument("Backdrop sigma must be finite and non-negative");
    setBackdropBlurKernel({value,value,0});
}

void BackdropEffect::setBackdropBlurKernel(BackdropBlurKernel kernel) {
    _backdropBlurKernel = validatedKernel(kernel);
    _backdropPyramid.reset();
    _backdropBlurRadius = std::max(_backdropBlurKernel.sigma0,_backdropBlurKernel.sigma1);
}

void BackdropEffect::setBackdropPyramid(NXFloat maximumRadius, std::vector<std::array<NXFloat,3>> taps,
    NXFloat minimumRadius,std::vector<std::array<NXFloat,3>> initialTaps) {
    if (!std::isfinite(maximumRadius) || maximumRadius < 0 || !std::isfinite(minimumRadius)
        || minimumRadius<0 || minimumRadius>maximumRadius)
        throw std::invalid_argument("Backdrop pyramid requires finite ordered nonnegative radii");
    const auto normalize=[](auto& values) {
        if(values.empty() || values.size()>32)throw std::invalid_argument("Backdrop pyramid requires1..32 taps");
        NXFloat total=0;
        for(const auto& tap:values) {
            if(!std::isfinite(tap[0])||!std::isfinite(tap[1])||!std::isfinite(tap[2])||tap[2]<0)
                throw std::invalid_argument("Backdrop pyramid taps must be finite with nonnegative weights");
            total+=tap[2];
        }
        if(!std::isfinite(total)||total<=0)throw std::invalid_argument("Backdrop pyramid weights must have positive finite sum");
        for(auto& tap:values)tap[2]/=total;
    };
    normalize(taps);if(!initialTaps.empty())normalize(initialTaps);
    _backdropPyramid=BackdropPyramid{maximumRadius,std::move(taps),minimumRadius,std::move(initialTaps)};
}

void BackdropEffect::setBackdropScale(NXFloat value) {
    if (!std::isfinite(value) || value <= 0 || value > 1)
        throw std::invalid_argument("Backdrop sampling scale must be in (0, 1]");
    _backdropScale = value;
}

void BackdropEffect::setOutputOutset(NXFloat value) {
    if (!std::isfinite(value) || value < 0)
        throw std::invalid_argument("Backdrop output outset must be finite and non-negative");
    _outputOutset = value;
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
