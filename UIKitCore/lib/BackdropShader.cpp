#include "BackdropShader.hpp"
#include "LayerCornerGeometry.hpp"
#include "include/effects/SkImageFilters.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkM44.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDevice.h"
#include "src/core/SkSpecialImage.h"
#include "include/effects/SkRuntimeEffect.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string_view>
#include <tuple>

using namespace NXKit;
using NXKit::Internal::BackdropShader;
using NXKit::Internal::BackdropPyramidImage;

namespace {

using UniformType = SkRuntimeEffect::Uniform::Type;

const SkRuntimeEffect::Uniform* findUniform(
    const SkRuntimeEffect& runtimeEffect,
    const std::string& name
) {
    return runtimeEffect.findUniform(name);
}

void validateAutomaticUniform(
    const SkRuntimeEffect& runtimeEffect,
    const char* name,
    UniformType requiredType
) {
    const auto* uniform = runtimeEffect.findUniform(name);
    if (!uniform) return;
    if (uniform->type != requiredType || uniform->isArray()) {
        throw std::invalid_argument(
            std::string("BackdropEffect automatic uniform '") + name + "' has the wrong type"
        );
    }
}

bool isFloatUniform(UniformType type) {
    switch (type) {
        case UniformType::kFloat:
        case UniformType::kFloat2:
        case UniformType::kFloat3:
        case UniformType::kFloat4:
        case UniformType::kFloat2x2:
        case UniformType::kFloat3x3:
        case UniformType::kFloat4x4:
            return true;
        default:
            return false;
    }
}

}

sk_sp<SkRuntimeEffect> BackdropShader::compile(const BackdropEffect& effect) {
    auto result = SkRuntimeEffect::MakeForShader(SkString(effect._shaderSource.c_str()));
    if (!result.effect) {
        throw std::invalid_argument(
            "BackdropEffect SkSL failed to compile: " + std::string(result.errorText.c_str())
        );
    }
    return std::move(result.effect);
}

void BackdropShader::validate(
    const BackdropEffect& effect,
    const SkRuntimeEffect& runtimeEffect
) {
    const auto* child = runtimeEffect.findChild(effect._backdropShaderName);
    if (!child || child->type != SkRuntimeEffect::ChildType::kShader) {
        throw std::invalid_argument(
            "BackdropEffect shader must declare uniform shader '"
            + effect._backdropShaderName + "'"
        );
    }
    const auto* sharpChild = runtimeEffect.findChild("sharpContent");
    const bool hasSharpBackdrop = sharpChild && effect._backdropShaderName != "sharpContent";
    if (runtimeEffect.children().size() != (hasSharpBackdrop ? 2 : 1) + effect._backdropInputs.size()
        || (hasSharpBackdrop && sharpChild->type != SkRuntimeEffect::ChildType::kShader)) {
        throw std::invalid_argument(
            "BackdropEffect shader inputs must match the declared backdrop inputs"
        );
    }
    for (const auto& [name, radius] : effect._backdropInputs) {
        const auto* input = runtimeEffect.findChild(name.c_str());
        if (!input || input->type != SkRuntimeEffect::ChildType::kShader)
            throw std::invalid_argument("Missing backdrop shader input '" + name + "'");
    }

    validateAutomaticUniform(runtimeEffect, "resolution", UniformType::kFloat2);
    validateAutomaticUniform(runtimeEffect, "lensCenter", UniformType::kFloat2);
    validateAutomaticUniform(runtimeEffect, "lensSize", UniformType::kFloat2);
    validateAutomaticUniform(runtimeEffect, "cornerRadius", UniformType::kFloat);
    validateAutomaticUniform(runtimeEffect, "cornerCurve", UniformType::kFloat);
    for (const auto* name : Internal::layerCornerShaderNames)
        validateAutomaticUniform(runtimeEffect, name, UniformType::kFloat4);
    validateAutomaticUniform(runtimeEffect, "externalShapeCoverage", UniformType::kFloat);
    validateAutomaticUniform(runtimeEffect, "contentScale", UniformType::kFloat);
    validateAutomaticUniform(runtimeEffect, "filterScale", UniformType::kFloat);
    validateAutomaticUniform(runtimeEffect, "maximumSampleRadius", UniformType::kFloat);
    if(effect.backdropPyramid() && (!findUniform(runtimeEffect,"backdropPyramidRect") || !findUniform(runtimeEffect,"backdropPyramidInfo")))
        throw std::invalid_argument("Backdrop pyramid shaders must declare float4 backdropPyramidRect and backdropPyramidInfo");
    validateAutomaticUniform(runtimeEffect, "backdropPyramidRect", UniformType::kFloat4);
    validateAutomaticUniform(runtimeEffect, "backdropPyramidInfo", UniformType::kFloat4);
    validateAutomaticUniform(runtimeEffect, "interactionIntensity", UniformType::kFloat);
    validateAutomaticUniform(runtimeEffect, "interactionLocation", UniformType::kFloat2);

    for (const auto& [name, components] : effect._uniforms) {
        const auto* uniform = findUniform(runtimeEffect, name);
        if (!uniform) {
            throw std::invalid_argument(
                "BackdropEffect shader does not declare uniform '" + name + "'"
            );
        }

        if (!isFloatUniform(uniform->type)
            || uniform->sizeInBytes() != components.size() * sizeof(NXFloat))
        {
            throw std::invalid_argument(
                "BackdropEffect uniform '" + name + "' has the wrong number or type of values"
            );
        }
    }
}

std::span<const NXFloat> BackdropShader::uniformValues(const BackdropEffect& effect, const std::string& name) {
    const auto found = effect._uniforms.find(name);
    return found == effect._uniforms.end() ? std::span<const NXFloat>() : found->second;
}

sk_sp<SkImageFilter> BackdropShader::capture(NXFloat scale) {
    if (scale >= 1) return nullptr;
    const auto tapCount=static_cast<int>(std::clamp(std::round(.5f/scale),1.f,4.f));
    static const auto runtime = [] {
        std::array<sk_sp<SkRuntimeEffect>,4> effects;
        for(int taps=1;taps<=4;++taps) {
            const auto source=std::string("const int captureTapCount=")+std::to_string(taps)+";\n"+R"sksl(
                uniform shader captureSource;
                uniform float captureTaps;
                half4 captureCell(float2 p) {
                    half4 sum=half4(0);
                    // Bilinear samples integrate pairs of source pixels. Native
                    // .5 capture needs 1x1 pair; .125 needs 4x4 pairs, not one point
                    // that aliases content narrower than its 8px capture footprint.
                    for(int y=0;y<captureTapCount;++y)for(int x=0;x<captureTapCount;++x) {
                        sum+=captureSource.eval(p+(float2(x,y)+.5)/captureTaps-.5);
                    }
                    return sum/half(captureTaps*captureTaps);
                }
                half4 main(float2 xy) {
                    float2 grid=floor(xy-.5)+.5, f=fract(xy-.5);
                    half4 first=captureCell(grid);
                    // Filter intermediates normally land exactly on this lattice.
                    // Preserve fractional reconstruction, but do not evaluate
                    // neighbours whose interpolation coefficient is exactly zero.
                    half4 top=f.x==0?first:mix(first,captureCell(grid+float2(1,0)),f.x);
                    if(f.y==0)return top;
                    half4 bottom=captureCell(grid+float2(0,1));
                    if(f.x!=0)bottom=mix(bottom,captureCell(grid+float2(1,1)),f.x);
                    return mix(top,bottom,f.y);
                }
            )sksl";
            auto result=SkRuntimeEffect::MakeForShader(SkString(source));
            if (!result.effect) throw std::runtime_error(result.errorText.c_str());
            effects[taps-1]=std::move(result.effect);
        }
        return effects;
    }();
    SkRuntimeShaderBuilder builder(runtime[tapCount-1]);
    builder.uniform("captureTaps")=static_cast<float>(tapCount);
    auto downsample=SkImageFilters::MatrixTransform(SkMatrix::Scale(scale,scale),
        SkSamplingOptions(SkFilterMode::kLinear),nullptr);
    return SkImageFilters::RuntimeShader(builder,2,"captureSource",std::move(downsample));
}

bool BackdropShader::canFreezePyramid(SkCanvas* canvas,NXFloat radiusScale) {
    if(!canvas||!std::isfinite(radiusScale)||radiusScale<=0)return false;
    const auto* device=SkCanvasPriv::TopDevice(canvas);if(!device)return false;
    if(!device->isClipRect()||device->isClipAntiAliased()
        ||device->imageInfo().alphaType()!=kPremul_SkAlphaType)return false;
    const auto matrix=SkMatrix::Concat(device->globalToDevice().asM33(),
        SkMatrix::Concat(canvas->getTotalMatrix(),SkMatrix::Scale(1/radiusScale,1/radiusScale)));
    return matrix.isTranslate()&&matrix.getTranslateX()==std::round(matrix.getTranslateX())
        &&matrix.getTranslateY()==std::round(matrix.getTranslateY());
}
sk_sp<SkImageFilter> BackdropShader::freezePrefix(SkCanvas* canvas,SkRect inputBounds,NXFloat radiusScale) {
    if(!canFreezePyramid(canvas,radiusScale))return nullptr;
    auto* device=SkCanvasPriv::TopDevice(canvas);
    const auto toDevice=SkMatrix::Concat(device->globalToDevice().asM33(),
        SkMatrix::Concat(canvas->getTotalMatrix(),SkMatrix::Scale(1/radiusScale,1/radiusScale)));
    SkMatrix inverse;const auto mapped=toDevice.mapRect(inputBounds);
    if(!toDevice.invert(&inverse)||!mapped.isFinite())return nullptr;
    SkIRect pixels;mapped.roundOut(&pixels);
    if(!pixels.intersect(device->bounds()))return nullptr;
    const auto frozen=device->snapSpecial(pixels,true);if(!frozen)return nullptr;
    const auto imageToFilter=SkMatrix::Concat(inverse,SkMatrix::Translate(pixels.left(),pixels.top()));
    return SkImageFilters::Shader(frozen->asShader(SkTileMode::kDecal,
        SkSamplingOptions(SkFilterMode::kLinear),imageToFilter,true));
}
BackdropPyramidImage BackdropShader::makePyramid(const BackdropEffect& effect,SkRect domain,NXFloat radiusScale,SkCanvas* canvas,
    sk_sp<SkImageFilter> prefix,bool omitBaseTile) {
    BackdropPyramidImage result;
    if(!effect.backdropPyramid())return result;
    const auto& description=*effect.backdropPyramid();
    const auto captureScale=effect.backdropScale();
    if(!domain.isFinite() || domain.isEmpty() || !std::isfinite(radiusScale) || radiusScale<=0)
        throw std::invalid_argument("Backdrop pyramid requires a finite nonempty domain and positive scale");
    const double maximum=std::clamp(double(description.maximumRadius)*double(radiusScale)*double(captureScale),description.maximumRadius>0?2.0:1.0,2048.0);
    const auto last=static_cast<int>(std::ceil(std::log2(maximum)));
    const auto alignment=static_cast<NXFloat>(1<<last);
    const auto left=std::floor(domain.left()*captureScale/alignment)*alignment;
    const auto top=std::floor(domain.top()*captureScale/alignment)*alignment;
    const auto width=std::max(alignment,std::ceil(domain.right()*captureScale/alignment)*alignment-left);
    const auto height=std::max(alignment,std::ceil(domain.bottom()*captureScale/alignment)*alignment-top);
    result.rect={left,top,width,height};result.info={width,height,captureScale,static_cast<NXFloat>(last)};
    // Tap count is topology, not an animated parameter. Specialize it so the
    // GPU does not execute a 32-way uniform branch/array lookup at every mip
    // pixel (the standard glass stencil has only thirteen taps).
    static thread_local std::map<size_t,sk_sp<SkRuntimeEffect>> runtimes;
    const auto program=[&](size_t count) {
        if(const auto found=runtimes.find(count);found!=runtimes.end())return found->second;
        auto compiled=SkRuntimeEffect::MakeForShader(SkString((
            "const int tapCount="+std::to_string(count)+";\n"+R"sksl(
            uniform shader previous;
            uniform float4 pyramidTaps[tapCount];
            half4 main(float2 p) {
                half4 result=half4(0);
                for(int i=0;i<tapCount;++i)
                    result+=previous.eval(p+pyramidTaps[i].xy)*half(pyramidTaps[i].z);
                return result;
            }
        )sksl").c_str()));
        if(!compiled.effect)throw std::runtime_error(compiled.errorText.c_str());
        return runtimes.emplace(count,std::move(compiled.effect)).first->second;
    };
    NXFloat reach=0;
    for(const auto* taps:{&description.taps,&description.initialTaps})for(const auto& tap:*taps)
        reach=std::max(reach,std::hypot(tap[0],tap[1]));
    result.sourceReach=(reach*(alignment-1)+alignment*.5f+1)/captureScale;
    auto* device=canvas?SkCanvasPriv::TopDevice(canvas):nullptr;
    if(device&&!prefix) {
        auto input=domain;input.outset(result.sourceReach,result.sourceReach);
        // Deferred image filters preserve the device sampling phase under
        // fractional and affine transforms. Only freeze an equivalent integer
        // pixel lattice; all other transforms retain the original filter DAG.
        prefix=freezePrefix(canvas,input,radiusScale);
    }
    const auto reduction=[&](const auto& taps,sk_sp<SkImageFilter> input) {
        SkRuntimeShaderBuilder builder(program(taps.size()));
        std::vector<NXFloat> values(taps.size()*4);
        for(size_t i=0;i<taps.size();++i) {
            const auto& t=taps[i];values[i*4]=t[0]*.5f;values[i*4+1]=t[1]*.5f;values[i*4+2]=t[2];
            reach=std::max(reach,std::hypot(t[0],t[1]));
        }
        builder.uniform("pyramidTaps").set(values.data(),values.size());
        return SkImageFilters::RuntimeShader(builder,reach*.5f,"previous",std::move(input));
    };
    const bool initial=double(description.minimumRadius)*radiusScale*captureScale>=2
        &&!description.initialTaps.empty();
    std::vector<sk_sp<SkImageFilter>> tiles;
    auto level=capture(captureScale);NXFloat divisor=1,y=1;
    const bool freezeLevels=bool(prefix)&&device;
    if(prefix)level=SkImageFilters::Compose(std::move(level),std::move(prefix));
    SkRect support=SkRect::MakeEmpty();
    if(freezeLevels) {
        const auto toDevice=SkMatrix::Concat(device->globalToDevice().asM33(),
            SkMatrix::Concat(canvas->getTotalMatrix(),SkMatrix::Scale(1/radiusScale,1/radiusScale)));
        SkMatrix inverse;
        if(toDevice.invert(&inverse)) {
            support=SkMatrix::Scale(captureScale,captureScale).mapRect(inverse.mapRect(SkRect::Make(device->bounds())));
            // The frozen prefix is decal outside the device. Include the
            // capture-cell footprint and bilinear reconstruction before
            // discarding any guaranteed-transparent work.
            support.outset(3,3);
        }
    }
    NXFloat atlasHeight=height+2;
    for(int i=0;i<=last;++i) {
        const auto w=width/divisor,h=height/divisor;
        if(freezeLevels) {
            // Freeze each reduction once, with enough halo for every remaining
            // stencil. Sharing a lazy filter DAG alone re-evaluates its prefix
            // for the different bounds requested by each packed atlas tile.
            auto bounds=SkRect::MakeXYWH(left/divisor,top/divisor,w,h);
            const auto halo=(reach+1)*(alignment/divisor-1)+2;
            bounds.outset(halo,halo);
            if(!support.isEmpty())bounds.intersect(support);
            const auto pixels=bounds.roundOut();
            const SkDevice::CreateInfo info(device->imageInfo().makeDimensions(pixels.size()),
                kUnknown_SkPixelGeometry,nullptr);
            auto intermediate=device->createDevice(info,nullptr);
            if(intermediate) {
                {
                    SkCanvas target(intermediate);target.clear(SK_ColorTRANSPARENT);
                    target.translate(-pixels.left(),-pixels.top());
                    SkPaint paint;paint.setBlendMode(SkBlendMode::kSrc);paint.setImageFilter(level);target.drawPaint(paint);
                }
                // A client Surface is unbudgeted and makeImageSnapshot copies
                // it on Graphite. An immutable child device is a reusable,
                // budgeted filter intermediate with a safe no-copy image view.
                intermediate->setImmutable();
                if(auto frozen=intermediate->snapSpecial(SkIRect::MakeSize(pixels.size()),false))
                    level=SkImageFilters::Image(frozen->asImage(),SkRect::Make(frozen->subset()),
                        SkRect::Make(pixels),SkSamplingOptions(SkFilterMode::kLinear));
            }
        }
        const auto x=i==0?1:width+3;
        const auto tileY=i==0?1:y;
        auto tile=SkImageFilters::Crop(SkRect::MakeLTRB(left/divisor,top/divisor,(left+width)/divisor,(top+height)/divisor),level);
        tile=SkImageFilters::MatrixTransform(SkMatrix::Translate(x-left/divisor,tileY-top/divisor),SkSamplingOptions(SkFilterMode::kLinear),tile);
        // Keep the capture and first reduction intact; only omit the unused
        // atlas copy. Tile coordinates and every sampled level stay unchanged.
        if(i!=0 || !omitBaseTile || !initial) tiles.push_back(std::move(tile));
        atlasHeight=std::max(atlasHeight,tileY+h+1);
        if(i>0)y+=h+2;
        if(i<last) {
            // The reduction transform remains deferred into the input shader.
            // Evaluate the stencil at destination resolution instead of
            // drawing a full-size blurred intermediate and shrinking it.
            level=SkImageFilters::MatrixTransform(SkMatrix::Scale(.5,.5),SkSamplingOptions(SkFilterMode::kLinear),level);
            level=reduction(i==0 && initial?description.initialTaps:description.taps,std::move(level));
            divisor*=2;
            // Convolution spills beyond its input. Propagate that finite
            // support through every reduction, including reconstruction;
            // clipping only to the viewport would lose valid edge energy.
            support=SkMatrix::Scale(.5,.5).mapRect(support);
            support.outset(reach*.5f+3,reach*.5f+3);
        }
    }
    result.image=SkImageFilters::Merge(tiles.data(),static_cast<int>(tiles.size()));
    result.sourceReach=(reach*(alignment-1)+alignment*.5f+1)/captureScale;
    result.sampleReach=std::max(width+width*.5f+4,atlasHeight)
        +std::max({std::abs(domain.left()),std::abs(domain.top()),std::abs(domain.right()),std::abs(domain.bottom())});
    return result;
}

sk_sp<SkImageFilter> BackdropShader::filteredInput(BackdropBlurKernel kernel,
    NXFloat scaleX, NXFloat scaleY, NXFloat captureScale, sk_sp<SkImageFilter> captured) {
    const auto gaussian = [&](NXFloat sigma) {
        return sigma > 0 ? SkImageFilters::Blur(sigma * scaleX * captureScale,
            sigma * scaleY * captureScale, SkTileMode::kClamp, captured) : captured;
    };
    auto result = gaussian(kernel.sigma0);
    if (kernel.weight > 0 && kernel.sigma0 != kernel.sigma1)
        result = SkImageFilters::Arithmetic(0, kernel.weight, 1-kernel.weight, 0, true,
            result, gaussian(kernel.sigma1));
    if (captureScale < 1) result = SkImageFilters::MatrixTransform(
        SkMatrix::Scale(1/captureScale,1/captureScale),SkSamplingOptions(SkFilterMode::kLinear),result);
    return result;
}

sk_sp<SkImageFilter> BackdropShader::makeFilter(
    const BackdropEffect& effect, const sk_sp<SkRuntimeEffect>& runtimeEffect,
    NXSize layerSize, NXFloat radius, CALayerCornerCurve curve, NXFloat scale,
    NXFloat intensity, NXPoint location, SkCanvas* canvas,bool omitBaseTile
) {
    SkRuntimeEffectBuilder builder(runtimeEffect);
    const auto size = SkV2{layerSize.width, layerSize.height};
    const auto filterScale = findUniform(*runtimeEffect, "filterScale")
        && std::isfinite(scale)
        ? std::max<NXFloat>(1, scale) : 1.0f;
    const auto finish=[&](sk_sp<SkImageFilter> filter) {
        if(filter && effect.outputBlurRadius()>0) {
            const auto sigma=effect.outputBlurRadius()*filterScale;
            return SkImageFilters::Blur(sigma,sigma,SkTileMode::kClamp,std::move(filter));
        }
        return filter;
    };

    if (findUniform(*runtimeEffect, "resolution")) {
        builder.uniform("resolution") = size;
    }
    if (findUniform(*runtimeEffect, "lensCenter")) {
        builder.uniform("lensCenter") = SkV2{size.x * 0.5f, size.y * 0.5f};
    }
    if (findUniform(*runtimeEffect, "lensSize")) {
        builder.uniform("lensSize") = size;
    }
    if (findUniform(*runtimeEffect, "cornerRadius")) {
        builder.uniform("cornerRadius") = std::isfinite(radius) && radius >= 0.0001f ? radius : 0.0f;
    }
    if (findUniform(*runtimeEffect, "cornerCurve")) {
        builder.uniform("cornerCurve") = curve == CALayerCornerCurve::continuous ? 1.0f : 0.0f;
    }
    Internal::bindLayerCornerGeometry(builder, *runtimeEffect, layerSize, radius);
    if (findUniform(*runtimeEffect, "externalShapeCoverage")) {
        builder.uniform("externalShapeCoverage") = effect.outputOutset() > 0 ? 0.0f : 1.0f;
    }
    if (findUniform(*runtimeEffect, "contentScale")) {
        builder.uniform("contentScale") = scale;
    }
    if (findUniform(*runtimeEffect, "filterScale")) {
        builder.uniform("filterScale") = filterScale;
    }
    if (findUniform(*runtimeEffect, "maximumSampleRadius")) {
        builder.uniform("maximumSampleRadius") = effect._maximumSampleRadius;
    }

    for (const auto& [name, components] : effect._uniforms) {
        builder.uniform(name).set(components.data(), static_cast<int>(components.size()));
    }

    if (findUniform(*runtimeEffect, "interactionIntensity")) {
        builder.uniform("interactionIntensity") = intensity;
    }
    if (findUniform(*runtimeEffect, "interactionLocation")) {
        builder.uniform("interactionLocation") = SkV2{location.x, location.y};
    }

    if(effect.backdropPyramid()) {
        const auto margin=(effect.maximumSampleRadius()+effect.outputOutset()+2)*filterScale;
        const auto domain=SkRect::MakeLTRB(-margin,-margin,size.x*filterScale+margin,size.y*filterScale+margin);
        const auto pyramid=makePyramid(effect,domain,filterScale,canvas,nullptr,omitBaseTile);
        if(findUniform(*runtimeEffect,"backdropPyramidRect"))builder.uniform("backdropPyramidRect").set(pyramid.rect.data(),4);
        if(findUniform(*runtimeEffect,"backdropPyramidInfo"))builder.uniform("backdropPyramidInfo").set(pyramid.info.data(),4);
        std::vector<std::string_view> names{effect.backdropShaderName()};
        std::vector<sk_sp<SkImageFilter>> images{pyramid.image};
        // RuntimeShader has one sampling radius for all children. Atlas
        // addressing can be much farther away than any optical source sample;
        // keep that address range from expanding the unfiltered scene input.
        if(runtimeEffect->findChild("sharpContent")){names.push_back("sharpContent");images.push_back(SkImageFilters::Crop(domain,nullptr));}
        auto captured=capture(effect.backdropScale());
        for(const auto& [name,kernel]:effect.backdropInputKernels()) {
            names.push_back(name);images.push_back(filteredInput(kernel,filterScale,filterScale,effect.backdropScale(),captured));
        }
        return finish(SkImageFilters::RuntimeShader(builder,std::max(pyramid.sampleReach,effect.maximumSampleRadius()*filterScale),names.data(),images.data(),static_cast<int>(names.size())));

    }
    if(findUniform(*runtimeEffect,"backdropPyramidRect"))builder.uniform("backdropPyramidRect")=SkV4{0,0,0,0};
    if(findUniform(*runtimeEffect,"backdropPyramidInfo"))builder.uniform("backdropPyramidInfo")=SkV4{0,0,1,-1};
    auto captured=capture(effect.backdropScale());
    const auto makeInput = [&](BackdropBlurKernel kernel) {
        return filteredInput(kernel,filterScale,filterScale,effect.backdropScale(),captured);
    };
    sk_sp<SkImageFilter> input = makeInput(effect._backdropBlurKernel);

    if (!effect._backdropInputs.empty()) {
        // Reuse equal kernels in the DAG. All children sample the same original
        // backdrop, never another child's already-composited output.
        using Key = std::tuple<NXFloat,NXFloat,NXFloat>;
        const auto key = [](BackdropBlurKernel k) { return Key{k.sigma0,k.sigma1,k.weight}; };
        std::map<Key, sk_sp<SkImageFilter>> kernels {{key(effect._backdropBlurKernel), input}};
        std::vector<std::string_view> names {effect._backdropShaderName};
        std::vector<sk_sp<SkImageFilter>> inputs {input};
        for (const auto& [name, kernel] : effect._backdropInputKernels) {
            auto [it, inserted] = kernels.try_emplace(key(kernel));
            if (inserted) it->second = makeInput(kernel);
            names.push_back(name); inputs.push_back(it->second);
        }
        if (runtimeEffect->findChild("sharpContent") && effect._backdropShaderName != "sharpContent") {
            names.push_back("sharpContent"); inputs.push_back(nullptr);
        }
        return finish(SkImageFilters::RuntimeShader(builder, effect._maximumSampleRadius * filterScale,
            names.data(), inputs.data(), static_cast<int>(inputs.size())));
    }

    sk_sp<SkImageFilter> imageFilter;
    if (runtimeEffect->findChild("sharpContent") && effect._backdropShaderName != "sharpContent") {
        // Lens folds and the half-point rim need the original scene. Blurring
        // the only input first erases those details before refraction can run.
        std::string_view children[] = {effect._backdropShaderName, "sharpContent"};
        const sk_sp<SkImageFilter> inputs[] = {std::move(input), nullptr};
        imageFilter = SkImageFilters::RuntimeShader(
            builder, effect._maximumSampleRadius * filterScale, children, inputs, 2
        );
    } else {
        imageFilter = SkImageFilters::RuntimeShader(
            builder, effect._maximumSampleRadius * filterScale, effect._backdropShaderName, std::move(input)
        );
    }
    return finish(std::move(imageFilter));
}
