#pragma once
#include <UIGlassEffect.h>
#include "LayerCornerGeometry.hpp"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include <array>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
#include "include/core/SkSurface.h"
#include <stdexcept>
#include <string>

// Exercise the actual material program without moving its geometry. A press
// may change the glass body; the same exterior backdrop/shadow must remain
// unchanged. Constant children isolate this from capture/filter differences.
template<class Backend, class Resolve>
void glassPressCoverageFixture(Backend& backend, Resolve resolve) {
    const auto source=NXKit::UIGlassEffect().backdropEffect().shaderSource();
    auto compiled=SkRuntimeEffect::MakeForShader(SkString(source));
    if(!compiled.effect)throw std::runtime_error(compiled.errorText.c_str());
    const auto runtime=compiled.effect;
    int cases=0,changedBodies=0;
    for(const NXKit::NXSize size:{NXKit::NXSize{52,52},{132,56},{398,62}})
    for(bool dark:{false,true})for(int scale:{1,2,3})
    for(const auto style:{NXKit::UIGlassEffect::Style::clear,NXKit::UIGlassEffect::Style::regular})
    for(bool tinted:{false,true})for(float amount:{0.f,.5f,1.f})
    for(float strength:{.25f,1.f}) {
        NXKit::UIGlassEffect material(style);
        if(tinted)material.setTintColor(NXKit::UIColor::systemBlue);
        const auto description=resolve(material,size,dark,scale,amount);
        const float w=size.width,h=size.height;
        const std::array<SkPoint,10> points={SkPoint{-4,h*.5f},{w+4,h*.5f},{w*.5f,-4},{w*.5f,h+4},
            {0,0},{w,0},{0,h},{w,h},{w*.5f,h*.5f},{w*.5f,3}};
        std::array<std::vector<uint8_t>,2> pixels;
        for(int pressed=0;pressed<2;++pressed) {
            SkRuntimeShaderBuilder builder(runtime);
            for(const auto& uniform:runtime->uniforms()) {
                if(const auto value=description.uniform(std::string(uniform.name)))
                    builder.uniform(uniform.name).set(value->data(),value->size());
            }
            builder.uniform("resolution")=SkV2{w,h};builder.uniform("lensSize")=SkV2{w,h};
            builder.uniform("lensCenter")=SkV2{w*.5f,h*.5f};
            builder.uniform("cornerRadius")=std::min(w,h)*.5f;builder.uniform("cornerCurve")=1.f;
            NXKit::Internal::bindLayerCornerGeometry(builder,*runtime,size,std::min(w,h)*.5f);
            builder.uniform("contentScale")=float(scale);builder.uniform("filterScale")=1.f;
            builder.uniform("externalShapeCoverage")=0.f;
            builder.uniform("maximumSampleRadius")=description.maximumSampleRadius();
            builder.uniform("interactionIntensity")=pressed?strength:0.f;
            builder.uniform("interactionLocation")=SkV2{w*.5f,h*.5f};
            builder.uniform("backdropPyramidRect")=SkV4{0,0,1,1};
            builder.uniform("backdropPyramidInfo")=SkV4{1,1,1,-1};
            const SkColor background=dark?SkColorSetRGB(36,43,55):SkColorSetRGB(187,174,158);
            for(const auto& child:runtime->children())builder.child(child.name)=SkShaders::Color(background);
            SkPaint paint;paint.setShader(builder.makeShader());
            auto surface=backend.surface(static_cast<int>(points.size()),1);auto* canvas=surface->getCanvas();
            canvas->clear(SK_ColorTRANSPARENT);
            for(size_t i=0;i<points.size();++i) {
                const auto p=points[i];canvas->save();canvas->translate(i+.5f-p.x(),.5f-p.y());
                canvas->drawRect(SkRect::MakeXYWH(p.x()-.5f,p.y()-.5f,1,1),paint);canvas->restore();
            }
            pixels[pressed]=backend.read(surface);
        }
        for(size_t i=0;i<8*4;++i)if(pixels[0][i]!=pixels[1][i])
            throw std::runtime_error("Press lighting changes exterior glass pixels: case="+std::to_string(cases)
                +" probe="+std::to_string(i/4)+" before="+std::to_string(pixels[0][i])+" held="+std::to_string(pixels[1][i]));
        if(!std::equal(pixels[0].begin()+8*4,pixels[0].end(),pixels[1].begin()+8*4))++changedBodies;
        ++cases;
    }
    if(changedBodies<cases/2)throw std::runtime_error("Press coverage fixture lost the material's interior response");
    std::cout<<"Glass press coverage: "<<cases<<" cases, unchanged exterior; "<<changedBodies<<" responsive bodies\n";
}
