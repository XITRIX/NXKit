#pragma once

#include <Geometry.h>
#include <UIColor.h>

#include <array>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace NXKit {

namespace Internal { class BackdropShader; }

/**
 Describes an SkSL effect that samples pixels behind a CABackdropLayer.

 The shader must declare a child shader whose name matches backdropShaderName().
 NXKit supplies that child with the optionally blurred live backdrop. An
 optional second child named `sharpContent` receives the original, unfiltered
 backdrop for sharp refraction and rim sampling. The following uniforms are
 optional and, when declared, are populated automatically in view-local points:

 - `resolution`: the view size
 - `lensCenter`: the center of the view
 - `lensSize`: the view size
 - `cornerRadius`: the backing layer's corner radius
 - `cornerCurve`: 0 for circular, 1 for continuous corners (NXKit's default)
 - `externalShapeCoverage`: 1 when the host's antialiased clip supplies edge
   coverage; shaders should then return fully shaded pixels at the perimeter
 - `contentScale`: the backing layer's pixel scale
 - `filterScale`: opt in to a pixel-resolution image filter; divide main's
   coordinates by this scale and multiply child-sampling coordinates by it
 - `maximumSampleRadius`: the effect's declared backdrop sampling radius
 - `interactionIntensity`: the hosting glass view's animated press light, 0...1
 - `interactionLocation`: the touch location relative to the material origin

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

    [[nodiscard]] const std::string& shaderSource() const { return _shaderSource; }
    [[nodiscard]] const std::string& backdropShaderName() const { return _backdropShaderName; }
    [[nodiscard]] NXFloat maximumSampleRadius() const { return _maximumSampleRadius; }
    [[nodiscard]] NXFloat backdropBlurRadius() const { return _backdropBlurRadius; }
    [[nodiscard]] std::optional<std::vector<NXFloat>> uniform(
        const std::string& name
    ) const;

    void setUniform(const std::string& name, NXFloat value);
    void setUniform(const std::string& name, NXPoint value);
    void setUniform(const std::string& name, NXSize value);
    void setUniform(const std::string& name, const std::array<NXFloat, 3>& value);
    void setUniform(const std::string& name, const std::array<NXFloat, 4>& value);
    void setUniform(const std::string& name, const std::vector<NXFloat>& value);
    void setUniform(const std::string& name, const UIColor& value);
    void removeUniform(const std::string& name);

private:
    friend class Internal::BackdropShader;

    static bool isAutomaticUniform(const std::string& name);
    void setFloatUniform(const std::string& name, std::vector<NXFloat> value);

    std::string _shaderSource;
    std::string _backdropShaderName;
    NXFloat _maximumSampleRadius;
    NXFloat _backdropBlurRadius;
    std::map<std::string, std::vector<NXFloat>> _uniforms;
};

}
