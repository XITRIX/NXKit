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

/** Portable Gaussian-kernel mixture. Mixing two filtered images retains one
    shader input and preserves source alpha; weight is the second kernel's share. */
struct BackdropBlurKernel {
    NXFloat sigma0 = 0, sigma1 = 0, weight = 0;
    bool operator==(const BackdropBlurKernel&) const = default;
};

/** Generic reduced-resolution filter pyramid. Each tap is {x,y,weight} in
    preceding-level pixels. maximumRadius is in local points and bounds levels. */
struct BackdropPyramid {
    NXFloat maximumRadius = 0;
    std::vector<std::array<NXFloat,3>> taps;
    NXFloat minimumRadius = 0;
    // Optional first-reduction stencil when the minimum footprint spans at
    // least two captured pixels. Later reductions always use taps.
    std::vector<std::array<NXFloat,3>> initialTaps;
};

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
    /** Maximum Gaussian sigma used by the primary lane. */
    [[nodiscard]] NXFloat backdropBlurRadius() const { return _backdropBlurRadius; }
    void setBackdropBlurRadius(NXFloat radius);
    void setBackdropBlurKernel(BackdropBlurKernel kernel);
    /** Rendering extension: packed mip pyramid in the primary child. Shaders
        must declare float4 backdropPyramidRect={sourceX,sourceY,width,height}
        and backdropPyramidInfo={width,height,captureScale,lastLevel}.
        Rect and dimensions use captured pixels. Level0 starts at atlas(1,1);
        later levels stack vertically at x=width+3 with two pixels between
        tiles. Sample each level at (physicalXY*captureScale-sourceXY)/2^L,
        clamp within its half-texel inset, and interpolate adjacent levels.
        A Gaussian override clears this descriptor; named inputs stay independent. */
    void setBackdropPyramid(NXFloat maximumRadius, std::vector<std::array<NXFloat,3>> taps,
        NXFloat minimumRadius=0, std::vector<std::array<NXFloat,3>> initialTaps={});
    [[nodiscard]] const std::optional<BackdropPyramid>& backdropPyramid() const { return _backdropPyramid; }
    [[nodiscard]] BackdropBlurKernel backdropBlurKernel() const { return _backdropBlurKernel; }
    /** Resolution of filtered/captured inputs relative to the backing surface.
        sharpContent always retains the original pixels for composition. */
    void setBackdropScale(NXFloat scale);
    [[nodiscard]] NXFloat backdropScale() const { return _backdropScale; }
    /** Rendering extension: additional named samples of the same live backdrop,
        each with a Gaussian sigma or normalized mixture in local points. Zero supplies sharp
        input. The primary child and reserved sharpContent remain automatic. */
    void setBackdropInput(std::string shaderName, NXFloat blurRadius);
    void setBackdropInput(std::string shaderName, BackdropBlurKernel kernel);
    [[nodiscard]] const std::map<std::string, BackdropBlurKernel>& backdropInputKernels() const { return _backdropInputKernels; }
    [[nodiscard]] const std::map<std::string, NXFloat>& backdropInputs() const { return _backdropInputs; }
    /** Output extent outside the host bounds. A clipping host still clips it. */
    void setOutputOutset(NXFloat outset);
    [[nodiscard]] NXFloat outputOutset() const { return _outputOutset; }
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
    BackdropBlurKernel _backdropBlurKernel;
    std::optional<BackdropPyramid> _backdropPyramid;
    NXFloat _outputOutset = 0;
    NXFloat _backdropScale = 1;
    std::map<std::string, NXFloat> _backdropInputs;
    std::map<std::string, BackdropBlurKernel> _backdropInputKernels;
    std::map<std::string, std::vector<NXFloat>> _uniforms;
};

}
