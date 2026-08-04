#pragma once

#include <Geometry.h>
#include <UIColor.h>

#include <array>
#include <map>
#include <string>
#include <vector>

namespace NXKit {

class CABackdropEffectLayer;

/**
 Describes an SkSL effect that samples the pixels behind a BackdropEffectView.

 The shader must declare one child shader whose name matches backdropShaderName().
 NXKit supplies that child with the live backdrop. The following uniforms are
 optional and, when declared, are populated automatically in view-local points:

 - `resolution`: the view size
 - `lensCenter`: the center of the view
 - `lensSize`: the view size
 - `cornerRadius`: the backing layer's corner radius
 - `contentScale`: the backing layer's pixel scale
 - `maximumSampleRadius`: the effect's declared backdrop sampling radius

 Other float, vector, matrix, or float-array uniforms can be supplied with
 setUniform(). Automatic uniforms cannot be overridden.
 */
class BackdropEffect {
public:
    explicit BackdropEffect(
        std::string shaderSource,
        std::string backdropShaderName = "content",
        NXFloat maximumSampleRadius = 0,
        NXFloat backdropBlurRadius = 0
    );

    /**
     A rounded liquid-glass lens with refraction, chromatic dispersion, tint,
     and a live backdrop-colored specular reflection along its rim. Achromatic
     backdrops receive directional fallback lighting and adaptive body contrast
     so the default glass remains visible over both black and white content.
     */
    static BackdropEffect glass();

    [[nodiscard]] const std::string& shaderSource() const { return _shaderSource; }
    [[nodiscard]] const std::string& backdropShaderName() const { return _backdropShaderName; }
    [[nodiscard]] NXFloat maximumSampleRadius() const { return _maximumSampleRadius; }
    [[nodiscard]] NXFloat backdropBlurRadius() const { return _backdropBlurRadius; }

    void setUniform(const std::string& name, NXFloat value);
    void setUniform(const std::string& name, NXPoint value);
    void setUniform(const std::string& name, NXSize value);
    void setUniform(const std::string& name, const std::array<NXFloat, 3>& value);
    void setUniform(const std::string& name, const std::array<NXFloat, 4>& value);
    void setUniform(const std::string& name, const std::vector<NXFloat>& value);
    void setUniform(const std::string& name, const UIColor& value);
    void removeUniform(const std::string& name);

private:
    friend class CABackdropEffectLayer;

    static bool isAutomaticUniform(const std::string& name);
    void setFloatUniform(const std::string& name, std::vector<NXFloat> value);

    std::string _shaderSource;
    std::string _backdropShaderName;
    NXFloat _maximumSampleRadius;
    NXFloat _backdropBlurRadius;
    std::map<std::string, std::vector<NXFloat>> _uniforms;
};

}
