#pragma once

#include <CALayerCornerCurve.h>
#include <Geometry.h>
#include <array>
#include "include/core/SkPath.h"

namespace NXKit::Internal {

// Rounded-superellipse geometry shared by paths, hit distances and shaders.
// Parameter reconstruction adapted from Flutter's BSD-licensed implementation;
// see licenses/Flutter-BSD-3-Clause.txt and the corner comparison report.
SkPath layerCornerPath(NXSize size, NXFloat radius, CALayerCornerCurve curve);
NXFloat layerCornerDistance(NXPoint centeredPoint, NXSize size, NXFloat radius, CALayerCornerCurve curve);

using LayerCornerShaderData = std::array<std::array<NXFloat,4>,4>;
LayerCornerShaderData layerCornerShaderData(NXSize size, NXFloat radius);
const char* layerCornerShaderSource();
inline constexpr std::array<const char*,4> layerCornerShaderNames = {
    "cornerTop", "cornerTopCircle", "cornerRight", "cornerRightCircle"
};
inline constexpr const char* layerCornerShaderUniforms =
    "uniform float4 cornerTop;\nuniform float4 cornerTopCircle;\n"
    "uniform float4 cornerRight;\nuniform float4 cornerRightCircle;\n";

template<class Builder, class Runtime>
void bindLayerCornerGeometry(Builder& builder, const Runtime& runtime, NXSize size, NXFloat radius) {
    if (!runtime.findUniform("cornerTop")) return;
    const auto data = layerCornerShaderData(size,radius);
    for (size_t i=0;i<layerCornerShaderNames.size();++i)
        if (runtime.findUniform(layerCornerShaderNames[i]))
            builder.uniform(layerCornerShaderNames[i]).set(data[i].data(),4);
}

}
