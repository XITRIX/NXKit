#include "LensMaterial.hpp"
#include "NativeLensProfile.hpp"
#include "NativeLensShader.hpp"
#include "NativeGlassShader.hpp"
#include "GlassVibrancy.hpp"
#include "GlassForeground.hpp"
#include "LensForegroundComposition.hpp"
#include "src/shaders/SkPictureShader.h"
#include "src/core/SkSpecialImage.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include <typeinfo>
#include "include/effects/SkImageFilters.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkRuntimeEffect.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <numbers>

#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPicture.h"
#include "LayerCornerGeometry.hpp"

namespace NXKit::Internal {
namespace {
// These are frame-local filter inputs, never client-owned render surfaces.
// Child devices can be frozen without Graphite's client-surface snapshot copy.
// Retain the surface path for recording backends and regression controls.
template<class Draw>
sk_sp<SkShader> lensInputShader(SkCanvas* canvas,SkIRect pixels,const SkMatrix& imageToOwner,Draw&& draw) {
  static const bool clientSurface=[] {const auto value=std::getenv("NXKIT_LENS_SURFACE_INPUTS");return value&&value[0]=='1';}();
  const auto info=SkImageInfo::MakeN32Premul(pixels.width(),pixels.height(),canvas->imageInfo().refColorSpace());
  auto* device=SkCanvasPriv::TopDevice(canvas);
  if(!clientSurface&&device&&!device->isNoPixelsDevice()) {
    auto child=device->createDevice(SkDevice::CreateInfo(info,kUnknown_SkPixelGeometry,nullptr),nullptr);
    if(child) {
      {SkCanvas target(child);draw(&target);}
      child->setImmutable();
      if(const auto image=child->snapSpecial(SkIRect::MakeWH(pixels.width(),pixels.height()),false)) {
        if(auto* metrics=RenderPerformance::current)++metrics->immutableLensInputs;
        return image->asShader(SkTileMode::kDecal,SkSamplingOptions(SkFilterMode::kLinear),imageToOwner,true);
      }
    }
  }
  auto surface=canvas->makeSurface(info);if(!surface)return nullptr;
  draw(surface->getCanvas());
  const auto image=surface->makeImageSnapshot();
  return image?image->makeShader(SkTileMode::kDecal,SkTileMode::kDecal,SkSamplingOptions(SkFilterMode::kLinear),&imageToOwner):nullptr;
}
}
void LensLayer::setSource(const std::shared_ptr<UIView>& source, const std::shared_ptr<UIView>& owner) {
  _source = source; _owner = owner;
  if (auto active = presentation()) {
    const auto lens = std::static_pointer_cast<LensLayer>(active);
    lens->_source = source; lens->_owner = owner;
  }
  setNeedsDisplay();
}
void LensLayer::drawSource(SkCanvas* canvas,bool isolateForeground,bool cropSource) {
  const auto sourceView = _source.lock(), ownerView = _owner.lock();
  if (!sourceView || !ownerView) return;
  // Source callbacks and nested lenses may reenter capture; cross-linked
  // sources must never recurse through A -> B -> A.
  static thread_local std::vector<const UIView*> activeOwners;
  if (std::find(activeOwners.begin(),activeOwners.end(),ownerView.get()) != activeOwners.end()) return;
  activeOwners.push_back(ownerView.get());
  struct Restore { std::vector<const UIView*>& owners; ~Restore() { owners.pop_back(); } } restore{activeOwners};
  struct RestoreEnvironment { std::shared_ptr<UIView> owner; ~RestoreEnvironment() { owner->updateCurrentEnvironment(); } } environment{ownerView};
  sourceView->drawAndLayoutTreeIfNeeded();
  std::vector<std::shared_ptr<UIView>> sourceChain, ownerChain;
  for (auto view = sourceView; view; view = view->superview().lock()) sourceChain.push_back(view);
  for (auto view = ownerView; view; view = view->superview().lock()) ownerChain.push_back(view);
  std::shared_ptr<UIView> common;
  for (const auto& view : sourceChain) if (std::find(ownerChain.begin(),ownerChain.end(),view) != ownerChain.end()) { common = view; break; }
  if (!common || common == sourceView) return;
  NXFloat ancestorAlpha = 1;
  for (auto view = sourceView->superview().lock(); view && view != common; view = view->superview().lock()) {
    const auto layer = view->layer()->presentationOrSelf();
    if (layer->isHidden()) return;
    ancestorAlpha *= layer->opacity();
  }
  if (ancestorAlpha < .001f) return;
  const auto ownMatrix = [](const std::shared_ptr<CALayer>& layer) {
    auto matrix = CATransform3DMakeTranslation(layer->_position.x,layer->_position.y,layer->_zPosition).concat(layer->_transform).toSkM44().asM33();
    matrix.preTranslate(-layer->_bounds.width()*layer->_anchorPoint.x,-layer->_bounds.height()*layer->_anchorPoint.y);
    matrix.preConcat(NXTransform3DMakeAffineTransform(layer->renderingTransform()).toSkM44().asM33());
    return matrix;
  };
  const auto toCommon = [&](const std::shared_ptr<UIView>& start, SkMatrix& result) {
    result = SkMatrix::I();
    for (auto view = start; view != common;) {
      const auto parent = view->superview().lock();
      if (!parent) return false;
      const auto layer = view->layer()->presentationOrSelf(), parentLayer = parent->layer()->presentationOrSelf();
      const auto transform = layer->transform();
      if (transform.m13 != 0 || transform.m23 != 0 || transform.m31 != 0 || transform.m32 != 0 || transform.m14 != 0 || transform.m24 != 0 || transform.m34 != 0 || transform.m44 != 1) return false;
      auto local = SkMatrix::Translate(-parentLayer->bounds().origin.x,-parentLayer->bounds().origin.y);
      local.preConcat(ownMatrix(layer));
      result = SkMatrix::Concat(local,result);
      view = parent;
    }
    return result.isFinite() && !result.hasPerspective();
  };
  SkMatrix sourceToCommon, ownerToCommon, commonToOwner;
  if (!toCommon(sourceView,sourceToCommon) || !toCommon(ownerView,ownerToCommon) || !ownerToCommon.invert(&commonToOwner)) return;
  const auto sourceLayer = sourceView->layer()->presentationOrSelf();
  const auto sourceSize = sourceLayer->bounds().size;
  if (!std::isfinite(sourceSize.width) || !std::isfinite(sourceSize.height) || sourceSize.width <= 0 || sourceSize.height <= 0) return;
  if (sourceLayer->isHidden() || sourceLayer->opacity() < .001f) return;
  // Strip output-only state on a detached root copy. Using setMask(nullptr)
  // here would mutate the real shared mask's parent; descendants stay intact.
  auto source = sourceLayer->copy();
  source->_mask.reset(); source->_masksToBounds = false; source->_lensSourceOwners.reset();
  SkMatrix inverseRoot;
  if (!ownMatrix(sourceLayer).invert(&inverseRoot)) return;
  auto recordingBounds = SkRect::MakeWH(sourceLayer->bounds().width(),sourceLayer->bounds().height());
  recordingBounds.outset(64,64);
  // Record the full source before the capsule is clipped. Zero-based picture
  // tiles preserve negative source overscan on both raster and GPU backends.
  SkPictureRecorder recorder;
  auto recording = recorder.beginRecording(SkRect::MakeWH(recordingBounds.width(),recordingBounds.height()));
  recording->translate(-recordingBounds.left(),-recordingBounds.top());
  recording->concat(inverseRoot);
  source->skiaRender(recording);
  const auto picture = recorder.finishRecordingAsPicture();
  const auto sourceToOwner = SkMatrix::Concat(commonToOwner,sourceToCommon);
  auto pictureToOwner = sourceToOwner; pictureToOwner.preTranslate(recordingBounds.left(),recordingBounds.top());
  if(_vibrant && _progress==0) {
    // No optical displacement at rest: blend the picture directly instead of
    // allocating a source surface and filtering the painted platter each frame.
    SkPaint paint;
    auto sourceShader=picture->makeShader(SkTileMode::kDecal,SkTileMode::kDecal,SkFilterMode::kLinear,&pictureToOwner,nullptr);
    // Keep full source recording/callback semantics, but rasterize only pixels
    // the settled lens can sample. Unknown drawing and destination filters keep
    // the original full-picture path. Preserve Skia's original tile lattice,
    // including its size cap, rather than rescaling the cropped rectangle.
    static const bool fullSource=[] {const auto flag=std::getenv("NXKIT_FULL_LENS_SOURCE");return flag&&flag[0]=='1';}();
    const auto plain=[](auto&& recurse,const std::shared_ptr<CALayer>& layer)->bool {
      const auto node=layer->presentationOrSelf();const CALayer& concrete=*node;
      if(typeid(concrete)!=typeid(CALayer)||node->_mask||!node->_contentMasks.empty()||node->_cornerRadius>0||node->_configuredCornerRadii||node->transform()!=NXTransform3DIdentity
         ||(node->_borderColor&&node->_borderWidth>0)||(node->_shadowColor&&node->_shadowColor->a()>0&&node->_shadowOpacity>0))return false;
      for(const auto& child:node->_sublayers)if(!recurse(recurse,child))return false;
      return true;
    };
    auto* device=SkCanvasPriv::TopDevice(canvas);const auto matrix=canvas->getTotalMatrix();
    const auto total=SkMatrix::Concat(matrix,pictureToOwner);SkMatrix ownerToPicture;
    if(cropSource&&!fullSource&&device&&!device->isNoPixelsDevice()&&!canvas->recordingContext()
       &&matrix.isScaleTranslate()&&matrix.getScaleX()>0&&matrix.getScaleX()==matrix.getScaleY()
       &&total.isScaleTranslate()&&total.getScaleX()>0&&total.getScaleX()==total.getScaleY()
       &&pictureToOwner.invert(&ownerToPicture)&&plain(plain,source)) {
      const auto recorder=canvas->recorder();
      const auto props=recorder?SkSurfaceProps():device->surfaceProps();
      const auto info=SkPictureShader::CachedImageInfo::Make(picture->cullRect(),total,
          device->imageInfo().colorType(),device->imageInfo().colorSpace(),recorder?recorder->maxTextureSize():0,props);
      if(info.success) {
        auto needed=SkRect::MakeWH(bounds().width(),bounds().height());
        needed.outset(2/matrix.getScaleX(),2/matrix.getScaleY());
        auto sampleBounds=info.matrixForDraw.mapRect(ownerToPicture.mapRect(needed));
        sampleBounds.outset(1,1); // bilinear footprint on the original raster lattice
        auto pixels=sampleBounds.roundOut();
        if(pixels.intersect(SkIRect::MakeSize(info.imageInfo.dimensions()))
           &&pixels.width()*int64_t(pixels.height())<info.imageInfo.width()*int64_t(info.imageInfo.height())) {
          auto cropped=info;cropped.imageInfo=info.imageInfo.makeDimensions(pixels.size());
          cropped.matrixForDraw.postTranslate(-pixels.left(),-pixels.top());
          sk_sp<SkSurface> surface;
          if(recorder)surface=skgpu::graphite::Surface::Make(recorder,cropped.imageInfo,"LensSourceROI",
              skgpu::Budgeted::kYes,skgpu::Mipmapped::kNo,SkBackingFit::kExact,&cropped.props);
          else surface=SkSurfaces::Raster(cropped.imageInfo,&cropped.props);
          if(surface)if(auto image=cropped.makeImage(std::move(surface),picture.get())) {
            SkMatrix rasterToPicture;
            if(info.matrixForDraw.invert(&rasterToPicture)) {
              auto imageToOwner=SkMatrix::Concat(pictureToOwner,rasterToPicture);
              imageToOwner.preTranslate(pixels.left(),pixels.top());
              sourceShader=image->makeShader(SkTileMode::kDecal,SkTileMode::kDecal,SkSamplingOptions(SkFilterMode::kLinear),imageToOwner);
              if(auto* metrics=RenderPerformance::current){++metrics->croppedLensSources;metrics->lensSourcePixels+=uint64_t(pixels.width())*pixels.height();}
            }
          }
        }
      }
    }
    paint.setShader(std::move(sourceShader));
    paint.setBlender(glassForegroundBlender(_dark));
    paint.setAlphaf(std::clamp(ancestorAlpha,0.0f,1.0f));
    // A bounded initialized target avoids restarting fullscreen MSAA while
    // retaining destination-dependent vibrancy and the original path coverage.
    LensForegroundComposition composition(canvas,bounds().size,isolateForeground);
    canvas->save();
    canvas->clipPath(layerCornerPath(bounds().size,std::min(bounds().width(),bounds().height())*.5f,CALayerCornerCurve::continuous),true);
    canvas->drawRect(SkRect::MakeWH(bounds().width(),bounds().height()),paint);
    canvas->restore();return;
  }
  sk_sp<SkShader> shader;SkMatrix samplingToOwner=SkMatrix::I();SkIRect sourceDeviceRect;
  bool useVibrancy=_vibrant;NXFloat sourceScale=_vibrant?std::max<NXFloat>(1,contentsScale()):1;
  if(_vibrant) {
    // Resolve the source on the actual device pixel grid before the image
    // filter samples it. A Picture shader inside a runtime image filter can
    // cache a one-point tile or introduce a second fractional edge filter.
    // A compatible surface keeps GPU images on their rendering backend.
    const auto ownerToDevice=canvas->getTotalMatrix();SkMatrix deviceToOwner;
    if(!ownerToDevice.invert(&deviceToOwner))return;
    auto deviceRect=ownerToDevice.mapRect(SkRect::MakeWH(bounds().width(),bounds().height())).roundOut();deviceRect.outset(2,2);
    samplingToOwner=deviceToOwner;sourceDeviceRect=deviceRect;
    auto imageToOwner=deviceToOwner;imageToOwner.preTranslate(deviceRect.left(),deviceRect.top());imageToOwner.postScale(sourceScale,sourceScale);
    shader=lensInputShader(canvas,deviceRect,imageToOwner,[&](SkCanvas* input) {
      input->clear(SK_ColorTRANSPARENT);
      input->translate(-deviceRect.left(),-deviceRect.top());input->concat(ownerToDevice);
      input->drawPicture(picture,&pictureToOwner,nullptr);
    });
  }
  if(!shader) {
    // Recording canvases have no compatible render target. Preserve their
    // selected content with the normal path instead of recording a blank.
    useVibrancy=false;sourceScale=1;samplingToOwner=SkMatrix::I();
    shader=picture->makeShader(SkTileMode::kDecal,SkTileMode::kDecal,SkFilterMode::kLinear,&pictureToOwner,nullptr);
  }
  if (!shader) return;
  // Native line sweeps retain an identity center but fold selected source
  // bands inward near the perimeter. Keep the full picture shader here: an
  // additional Picture image-filter input changes fractional source phase.
  static const auto sourceEffects = [] {
    std::array<sk_sp<SkRuntimeEffect>,2> effects;
    for(int variant=0;variant<2;++variant) {
    auto compiled = SkRuntimeEffect::MakeForShader(SkString(std::string(variant?"const bool vibrantSource=true;\n":"const bool vibrantSource=false;\n") + "uniform float cornerCurve;\n" + layerCornerShaderUniforms + layerCornerShaderSource() + "uniform float contentScale;\n" + std::string(nativeLensShaderFunctions()) + glassVibrancySource + R"sksl(
uniform shader selected;
uniform shader opticalSelected;
uniform float bufferedOptics;
uniform shader scene;
uniform float3x3 samplingToOwner;
uniform float sourceScale;
uniform float dark;
uniform float2 size;
uniform float pass;
uniform float progress;
uniform float selection;
uniform float opacity;
uniform float sourceMagnification;
float2 magnifiedPosition(float2 p) {
  return sourceMagnification==1.0 ? p : size*.5+(p-size*.5)/sourceMagnification;
}
float capsule(float2 p) {
  return boxRoundedSDF(p-size*.5,size*.5,min(size.x,size.y)*.5);
}
float2 opticalNormal(float2 p) {
  return lensNormalDirection(p-size*.5,size*.5,min(size.x,size.y)*.5);
}
uniform float2 wrapperDisplacement;
uniform float2 sourceDisplacement;
uniform float4 foregroundRefraction;
uniform float4 foregroundAberration;
uniform float4 foregroundEdge;
uniform float foregroundCaptureMargin;
float2 sourcePosition(float2 p) {
  return magnifiedPosition(lensDisplaced(p,size,wrapperDisplacement.x,wrapperDisplacement.y,1,.5));
}
half4 sourceAt(float2 p) {
  float mask=clamp(.5-lensShape(p,size).x*max(contentScale,1),0,1);
  return selected.eval(sourcePosition(p)*sourceScale)*half(mask);
}
half4 displacedSourceAt(float2 p) {
  if(bufferedOptics>.5)return opticalSelected.eval(p*sourceScale);
  float mask=clamp(.5-lensShape(p,size).x*max(contentScale,1),0,1);
  return sourceAt(lensDisplaced(p,size,sourceDisplacement.x,sourceDisplacement.y,1,0))*half(mask);
}
void spectralSource(float2 p,out half3 ink,out half3 coverage) {
  float3 field=lensShape(p,size);
  float2 center=p+field.yz*foregroundRefraction.x*lensShoulder(-field.x,foregroundRefraction.y,foregroundRefraction.z);
  float2 spread=lensAberration(field.yz,foregroundAberration.w,lensForegroundExtent(size,foregroundCaptureMargin))*foregroundAberration.x
      *lensShoulder(-field.x,foregroundAberration.y,foregroundAberration.z);
  float3 rgb=float3(0),alpha=float3(0);
  for(int i=0;i<7;++i) {
    float position=1-float(i)/3;
    half4 sample=displacedSourceAt(center+spread*position);
    float r=max(position,0),b=max(-position,0),g=1-abs(position);
    float3 weight=float3(r,g,b)*float3(.5,1.0/3,.5);
    rgb+=float3(sample.rgb)*weight;alpha+=float(sample.a)*weight;
  }
  ink=half3(rgb);coverage=half3(alpha);
}
half4 vibrantSample(half4 source,half4 background) {
  if(source.a<=0)return half4(0);
  half3 color=half3(glassVibrantColor(float3(source.rgb/max(source.a,.0001)),float3(background.rgb/max(background.a,.0001)),dark));
  return half4(mix(source.rgb,color*source.a,background.a),source.a);
}
half4 main(float2 coordinate) {
  half4 background=half4(0);
  if(vibrantSource) { background=scene.eval(coordinate);coordinate=(samplingToOwner*float3(coordinate,1)).xy; }
  float2 p=coordinate;
  half4 original=selected.eval(magnifiedPosition(coordinate)*sourceScale);
  if(vibrantSource)original=vibrantSample(original,background);
  half3 ink=original.rgb;
  half3 coverage=original.aaa;
  if(progress>0.0) {
    half4 base=displacedSourceAt(p);
    half3 spectralInk=base.rgb,spectralCoverage=base.aaa;
    half edge=0;
    if(foregroundAberration.x!=0 || foregroundRefraction.x!=0) {
      spectralSource(p,spectralInk,spectralCoverage);
      edge=half(lensEdgeOpacity(lensShape(p,size).x,foregroundEdge));
    }
    // UIKit filters the already composed opaque surface. Our selected ink is
    // retained separately: carry each spectral channel's coverage so its
    // background contribution is not replaced with transparent black.
    half3 color=mix(base.rgb,spectralInk,edge);
    half3 alpha=mix(base.aaa,spectralCoverage,edge);
    if(vibrantSource) {
      half3 straight=color/max(alpha,half3(.0001));
      color=mix(color,half3(glassVibrantColor(float3(straight),float3(background.rgb/max(background.a,.0001)),dark))*alpha,background.a);
    }
    ink=mix(original.rgb,color,half(progress));
    coverage=mix(original.aaa,alpha,half(progress));
  }
  ink*=half(opacity); coverage*=half(opacity);
  half alpha=max(coverage.r,max(coverage.g,coverage.b));
  if(vibrantSource&&pass>1.5)return half4(ink+background.rgb*(half3(1)-coverage),alpha+background.a*(1-alpha));
  return pass<.5 ? half4(coverage,1) : half4(ink,alpha);
}
)sksl"));
    if (!compiled.effect) throw std::runtime_error(compiled.errorText.c_str());
    effects[variant]=compiled.effect;
    }
    return effects;
  }();
  const auto& sourceEffect=sourceEffects[useVibrancy?1:0];
  // Channel-dependent coverage must attenuate the destination separately;
  // one maximum-alpha SrcOver would darken uncovered channels around black
  // ink. Two direct draws implement inkRGB + dstRGB * (1 - coverageRGB).
  // The common capsule AA multiplies attenuation and addition once each;
  // it does not apply source-over occlusion twice. Alpha remains premultiplied.
  static const auto attenuate = [] {
    auto compiled=SkRuntimeEffect::MakeForBlender(SkString("half4 main(half4 src, half4 dst) { return half4(dst.rgb*(half3(1)-src.rgb),dst.a); }"));
    if (!compiled.effect) throw std::runtime_error(compiled.errorText.c_str());
    return SkRuntimeBlendBuilder(compiled.effect).makeBlender();
  }();
  static const auto add = [] {
    auto compiled=SkRuntimeEffect::MakeForBlender(SkString("half4 main(half4 src, half4 dst) { return half4(src.rgb+dst.rgb,src.a+dst.a*(1-src.a)); }"));
    if (!compiled.effect) throw std::runtime_error(compiled.errorText.c_str());
    return SkRuntimeBlendBuilder(compiled.effect).makeBlender();
  }();
  SkRuntimeShaderBuilder sourceBuilder(sourceEffect);
  sourceBuilder.child("selected")=shader;
  if(useVibrancy) {sourceBuilder.child("scene")=SkShaders::Color(SK_ColorTRANSPARENT);sourceBuilder.uniform("samplingToOwner")=samplingToOwner;}
  const auto profile=nativeLensProfile(_selection,_dark,bounds().size);
  // UIKit rasterizes the wrapper and source-displacement layers before the
  // foreground samples them. Inlining both nonlinear maps into each spectral
  // read skips those sampling boundaries and changes thin glyph coverage.
  sk_sp<SkShader> opticalShader;
  if(_progress>0) {
    static const auto opticalEffect=[] {
      auto compiled=SkRuntimeEffect::MakeForShader(SkString(std::string("uniform float cornerCurve;\n")+
          layerCornerShaderUniforms+layerCornerShaderSource()+"uniform float contentScale;\n"+
          std::string(nativeLensShaderFunctions())+R"sksl(
uniform shader source;
uniform float2 size;
uniform float2 displacement;
uniform float ovalization;
uniform float sourceScale;
uniform float magnification;
half4 main(float2 p) {
  float mask=clamp(.5-lensShape(p,size).x*max(contentScale,1),0,1);
  if(mask<=0)return half4(0);
  float2 sample=lensDisplaced(p,size,displacement.x,displacement.y,1,ovalization);
  sample=size*.5+(sample-size*.5)/magnification;
  return source.eval(sample*sourceScale)*half(mask);
}
)sksl"));
      if(!compiled.effect)throw std::runtime_error(compiled.errorText.c_str());
      return compiled.effect;
    }();
    const auto ownerToDevice=canvas->getTotalMatrix();SkMatrix deviceToOwner;
    auto pixels=ownerToDevice.mapRect(SkRect::MakeWH(bounds().width(),bounds().height())).roundOut();pixels.outset(2,2);
    // A resource bound, not layout geometry. Unsupported/recording surfaces
    // retain the analytic path rather than allocating an unbounded image.
    if(ownerToDevice.invert(&deviceToOwner) && pixels.width()>0 && pixels.height()>0 &&
       int64_t(pixels.width())*pixels.height()<=16*1024*1024) {
      auto input=shader;
      for(int stage=0;stage<2 && input;++stage) {
        SkRuntimeShaderBuilder builder(opticalEffect);builder.child("source")=input;
        builder.uniform("size")=SkV2{bounds().width(),bounds().height()};
        builder.uniform("cornerCurve")=1.f;
        bindLayerCornerGeometry(builder,*opticalEffect,bounds().size,std::min(bounds().width(),bounds().height())*.5f);
        builder.uniform("contentScale")=std::max<NXFloat>(1,contentsScale());
        const auto& warp=stage==0?profile.contentWrapper:profile.source;
        builder.uniform("displacement")=SkV2{warp.amount,warp.height};
        builder.uniform("ovalization")=warp.gradientOvalization;
        builder.uniform("sourceScale")=sourceScale;
        builder.uniform("magnification")=stage==0?1+(_magnification-1)*_progress:1.f;
        auto imageToOwner=deviceToOwner;imageToOwner.preTranslate(pixels.left(),pixels.top());imageToOwner.postScale(sourceScale,sourceScale);
        auto stageShader=builder.makeShader();
        input=lensInputShader(canvas,pixels,imageToOwner,[&](SkCanvas* target) {
          target->clear(SK_ColorTRANSPARENT);
          target->translate(-pixels.left(),-pixels.top());target->concat(ownerToDevice);
          SkPaint paint;paint.setShader(stageShader);
          target->drawPaint(paint); // SDF coverage owns the fractional silhouette.
        });
      }
      opticalShader=std::move(input);
    }
  }
  sourceBuilder.child("opticalSelected")=opticalShader?opticalShader:shader;
  sourceBuilder.uniform("bufferedOptics")=opticalShader?1.f:0.f;
  sourceBuilder.uniform("wrapperDisplacement")=SkV2{profile.contentWrapper.amount,profile.contentWrapper.height};
  sourceBuilder.uniform("sourceDisplacement")=SkV2{profile.source.amount,profile.source.height};
  const auto& foreground=profile.foreground;
  sourceBuilder.uniform("foregroundRefraction")=SkV4{foreground.refractionAmount,foreground.refractionHeight,foreground.refractionOffset,0};
  sourceBuilder.uniform("foregroundAberration")=SkV4{foreground.aberrationAmount,foreground.aberrationHeight,foreground.aberrationOffset,foreground.aberrationAngle};
  sourceBuilder.uniform("foregroundEdge")=SkV4{foreground.edgeStart,foreground.edgeEnd,foreground.edgeOpacityStart,foreground.edgeOpacityEnd};
  sourceBuilder.uniform("foregroundCaptureMargin")=foreground.captureMargin;
  sourceBuilder.uniform("contentScale")=std::max<NXFloat>(1,contentsScale());
  sourceBuilder.uniform("sourceScale")=sourceScale;
  if(useVibrancy)sourceBuilder.uniform("dark")=_dark?1.f:0.f;
  sourceBuilder.uniform("size")=SkV2{bounds().width(),bounds().height()};
  sourceBuilder.uniform("cornerCurve")=1.f;
  bindLayerCornerGeometry(sourceBuilder,*sourceEffect,bounds().size,std::min(bounds().width(),bounds().height())*.5f);
  sourceBuilder.uniform("progress")=_progress;
  sourceBuilder.uniform("sourceMagnification")=1+(_magnification-1)*_progress;
  sourceBuilder.uniform("selection")=_selection?1.f:0.f;
  sourceBuilder.uniform("opacity")=std::clamp(ancestorAlpha,0.0f,1.0f);
  SkPaint paint;
  canvas->save();
  canvas->clipPath(layerCornerPath(bounds().size,std::min(bounds().width(),bounds().height())*.5f,CALayerCornerCurve::continuous),true);
  if(useVibrancy) {
    sourceBuilder.uniform("pass")=2.f;
    // This compositor reads the scene at the current pixel only. Snapshot
    // that prefix directly instead of initializing another offscreen layer
    // and evaluating a zero-radius runtime image filter at restore time.
    // Keep the old filter path for recording/unsupported devices and as an
    // independent opt-in regression control.
    static const bool filterForeground=[] {const auto value=std::getenv("NXKIT_LENS_FILTER_FOREGROUND");return value&&value[0]=='1';}();
    auto* device=SkCanvasPriv::TopDevice(canvas);
    if(!filterForeground&&device&&device->imageInfo().alphaType()==kPremul_SkAlphaType) {
      const auto toDevice=device->globalToDevice().asM33();
      const auto toGlobal=device->deviceToGlobal().asM33();
      auto pixels=toDevice.mapRect(SkRect::Make(sourceDeviceRect)).roundOut();
      if(toDevice.isTranslate()&&toDevice.getTranslateX()==std::round(toDevice.getTranslateX())
         &&toDevice.getTranslateY()==std::round(toDevice.getTranslateY())&&pixels.intersect(device->bounds())) {
        if(const auto prefix=device->snapSpecial(pixels,true)) {
          auto imageToGlobal=toGlobal;imageToGlobal.preTranslate(pixels.left(),pixels.top());
          sourceBuilder.child("scene")=prefix->asShader(SkTileMode::kDecal,SkSamplingOptions(SkFilterMode::kLinear),imageToGlobal,true);
          paint.setShader(sourceBuilder.makeShader());paint.setBlendMode(SkBlendMode::kSrc);
          canvas->resetMatrix();canvas->drawRect(SkRect::Make(sourceDeviceRect),paint);canvas->restore();
          if(auto* metrics=RenderPerformance::current)++metrics->directLensForegrounds;
          return;
        }
      }
    }
    // pass2 already contains the sampled scene. Replace it rather than
    // adding a translucent backdrop for a second time at layer restore.
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setImageFilter(SkImageFilters::RuntimeShader(sourceBuilder,0,"scene",nullptr));
    // Keep the image filter itself on integer device pixels. Restoring an
    // owner-local fractional layer would bilinearly filter its ink again.
    canvas->resetMatrix();
    const auto rect=SkRect::Make(sourceDeviceRect);
    canvas->saveLayer(SkCanvas::SaveLayerRec(&rect,&paint,SkCanvas::kInitWithPrevious_SaveLayerFlag));
    canvas->restore();canvas->restore();return;
  }
  sourceBuilder.uniform("pass")=0.f;
  paint.setShader(sourceBuilder.makeShader()); paint.setBlender(attenuate);
  canvas->drawRect(SkRect::MakeWH(bounds().width(),bounds().height()),paint);
  sourceBuilder.uniform("pass")=1.f;
  paint.setShader(sourceBuilder.makeShader()); paint.setBlender(add);
  canvas->drawRect(SkRect::MakeWH(bounds().width(),bounds().height()),paint);
  canvas->restore();
}

LensLayer::LensLayer(LensLayer *source)
    : CALayer(source), _source(source->_source), _owner(source->_owner), _selection(source->_selection),
      _progress(source->_progress), _magnification(source->_magnification), _vibrant(source->_vibrant), _dark(source->_dark) {}
std::shared_ptr<CALayer> LensLayer::copy() {
  return new_shared<LensLayer>(this);
}
void LensLayer::setLiftState(bool selection, NXFloat progress, NXFloat magnification, bool vibrant, bool dark) {
  _selection = selection;
  _progress = progress;
  _magnification = magnification;
  _vibrant = vibrant;_dark = dark;
  if (auto active = presentation()) {
    auto lens = std::static_pointer_cast<LensLayer>(active);
    lens->_selection = selection;
    lens->_progress = progress;
    lens->_magnification = magnification;
    lens->_vibrant = vibrant;lens->_dark = dark;
  }
  setNeedsDisplay();
}
void LensLayer::draw(SkCanvas *canvas) {
  drawSource(canvas);
  if (_progress <= 0 || bounds().size.width <= 0 || bounds().size.height <= 0)
    return;
  // The recovered glass-background pass owns its exterior ring/shadow.
}
BackdropEffect lensBackdropMaterial(bool selection, NXFloat progress, NXSize size, std::optional<NXFloat> displacement) {
  static const auto source=std::string("uniform shader content;uniform shader sharpContent;uniform float2 resolution;uniform float filterScale;uniform float cornerCurve;uniform float contentScale;uniform float2 displacement;\n")
      +layerCornerShaderUniforms+layerCornerShaderSource()+std::string(nativeLensShaderFunctions())+R"sksl(
half4 main(float2 xy) {
    float scale=max(filterScale,1);
    float2 p=xy/scale;
    float2 sample=lensDisplaced(p,resolution,displacement.x,displacement.y,1,.5);
    return content.eval(sample*scale);
})sksl";
  const auto profile=nativeLensProfile(selection,false,size);
  const auto amount=displacement.value_or(profile.spec.unliftedDisplacement+(profile.spec.liftedDisplacement-profile.spec.unliftedDisplacement)*progress);
  const auto blur=std::max(0.f,profile.spec.unliftedBlurRadius*(1-progress));
  BackdropEffect effect(source,"content",std::abs(amount)+profile.backdrop.height+3*blur);
  effect.setOutputBlurRadius(blur);
  effect.setUniform("displacement",NXPoint(amount,profile.backdrop.height));
  return effect;
}
BackdropEffect lensMaterial(bool selection, NXFloat progress, bool dark, NXSize size, bool warpsContentBelow, std::optional<NXSize> controlReferenceSize, bool controlLifted) {
  static const std::string source=[] {
    std::string material(nativeGlassMaterialShaderSource());
    auto entry=material.find("half4 main(float2 filterXY)");
    if(entry==std::string::npos)throw std::runtime_error("Native glass material entry is unavailable");
    material.erase(entry);
    // CASDFOutputEffect min/max allocate SDF support; they are not a radial
    // alpha cutoff. Applying that cutoff to the capsule chops its shadow tail.
    const auto coverageStart=material.find("float nativeGlassOutputCoverage(float sdf) {");
    if(coverageStart==std::string::npos)throw std::runtime_error("Native glass output support entry is unavailable");
    const auto coverageEnd=material.find("\n}",coverageStart);
    material.replace(coverageStart,coverageEnd+2-coverageStart,"float nativeGlassOutputCoverage(float sdf) { return 1; }");
    // The backdrop warp precedes the native clear-glass passes. Preserve the
    // original sampler for the resting interpolation and exterior pixels.
    auto at=material.find("float2 nativeSampleXY(");
    material.insert(at,std::string(nativeLensShaderFunctions())+R"sksl(
uniform float2 lensBackdropDisplacement;
uniform float4 lensForegroundAberration;
uniform float4 lensForegroundEdge;
uniform float lensForegroundCaptureMargin;
uniform float2 lensInnerRefractionFraction;
uniform float3 lensControlRefraction;
uniform float lensProgress;
float4 lensRefraction() {
    if(lensControlRefraction.z>.5)return float4(lensControlRefraction.xy,0,0);
    float shortSide=min(resolution.x,resolution.y);
    return float4(lensInnerRefractionFraction.x*shortSide,min(20,lensInnerRefractionFraction.y*shortSide),0,0);
}
half4 lensBackdropAt(float2 coordinate) {
    float2 p=coordinate/max(filterScale,1);
    float mask=clamp(.5-lensShape(p,resolution).x*max(contentScale,1),0,1);
    if(mask<=0)return sharpContent.eval(coordinate);
    p=lensDisplaced(p,resolution,lensBackdropDisplacement.x,lensBackdropDisplacement.y,1,.5);
    return mix(sharpContent.eval(coordinate),sharpContent.eval(p*max(filterScale,1)),half(mask));
}
)sksl");
    // Only the material helper source reads through the warped sampler.
    const auto begin=material.find("float2 nativeSampleXY(");
    const std::string from="sharpContent.eval(",to="lensBackdropAt(";
    size_t cursor=begin;
    while((cursor=material.find(from,cursor))!=std::string::npos) {
        material.replace(cursor,from.size(),to);cursor+=to.size();
    }
    // A zero-radius native lens samples the warped backdrop directly. The
    // shared glass helper otherwise reads LOD0 of its unwarped pyramid, even
    // at radius0, bypassing the preceding displacement pass entirely.
    const std::string radiusEntry="half4 nativeAtRadius(float2 origin,float2 target,float radius) {";
    const auto radiusAt=material.find(radiusEntry);
    if(radiusAt==std::string::npos)throw std::runtime_error("Native glass sampling entry is unavailable");
    material.insert(radiusAt+radiusEntry.size(),"\n    if(radius<=0)return lensBackdropAt(nativeSampleXY(origin,target));\n");
    // Effect uniforms are retained across a bounds animation, whereas the
    // renderer's resolution follows presentation geometry on every frame.
    // Resolve size-dependent native inputs from that live geometry.
    const std::string refraction="nativeRefraction.";
    cursor=begin;
    while((cursor=material.find(refraction,cursor))!=std::string::npos) {
        material.replace(cursor,refraction.size(),"lensRefraction().");cursor+=17;
    }
    // UIKit's SDF highlight is a final sibling, after the displacement and
    // foreground backdrop passes. Do not refract or disperse its light.
    const std::string lighting="rgb=float3(nativeGlassHighlight(sdf,normal,worldNormal,half3(rgb)));";
    const auto lightingAt=material.find(lighting);
    if(lightingAt==std::string::npos)throw std::runtime_error("Native glass highlight entry is unavailable");
    material.replace(lightingAt,lighting.size(),"rgb=unlit;");
    material+=R"sksl(
half4 lensMaterialAt(float2 p) {
    float3 field=lensShape(p,resolution);float radius=min(resolution.x,resolution.y)*.5;
    float shadow=boxRoundedSDF(p-nativeShadowOffset-resolution*.5,resolution*.5,radius);
    float ring=boxRoundedSDF(p-float2(0,nativeRingShadowOffset)-resolution*.5,resolution*.5,radius);
    half4 background=lensBackdropAt(p*max(filterScale,1));
    float coverage=clamp(.5-field.x*max(contentScale,1),0,1);
    return nativeGlassSurface(p,field.x,field.yz,field.yz,radius,shadow,ring,background,coverage,max(contentScale,1));
}
half4 main(float2 filterXY) {
    float2 p=filterXY/max(filterScale,1);
    float3 field=lensShape(p,resolution);
    half4 original=sharpContent.eval(filterXY);
    if(nativeGlassOutputCoverage(field.x)<=0)return original;
    half4 base=lensMaterialAt(p);
    float shortSide=min(resolution.x,resolution.y);
    float bodyCoverage=clamp(.5-field.x*max(contentScale,1),0,1);
    float4 edgeInputs=lensForegroundEdge;edgeInputs.x*=shortSide;
    float edge=lensEdgeOpacity(field.x,edgeInputs)*bodyCoverage;
    if(lensForegroundAberration.x>0 && edge>0) {
        float2 spread=lensAberration(field.yz,lensForegroundAberration.w,lensForegroundExtent(resolution,lensForegroundCaptureMargin))*lensForegroundAberration.x*shortSide
            *lensShoulder(-field.x,lensForegroundAberration.y,lensForegroundAberration.z*shortSide);
        float3 rgb=float3(0);float alpha=0;
        for(int i=0;i<7;++i) {
            float position=1-float(i)/3;
            half4 sample=lensMaterialAt(p+spread*position);
            float3 color=float3(sample.rgb)/max(float(sample.a),.000001);
            rgb+=color*float3(max(position,0),1-abs(position),max(-position,0));
            alpha+=float(sample.a)/7;
        }
        rgb*=float3(.5,1.0/3,.5);alpha*=edge;
        base=half4(half3(rgb*alpha),half(alpha))+base*(1-half(alpha));
    }
    // The native ClearGlass material stays active. Only its final highlight
    // follows the complementary resting-fill opacity, not the whole surface.
    half3 lit=nativeGlassHighlight(field.x,field.yz,field.yz,half3(nativeUnpremul(base)))*base.a;
    base.rgb=mix(base.rgb,lit,half(lensProgress));
    float coverage=externalShapeCoverage>.5?1:clamp(.5-field.x*max(contentScale,1),0,1);
    // Exterior shadow/rim remain native, without clipping to the body.
    return field.x>0?base:mix(original,base,half(coverage));
}
)sksl";
    return material;
  }();
  auto profile=nativeLensProfile(selection,dark,controlReferenceSize.value_or(size));
  {
    // Actual small and large lens CAFilter traces retire the material inputs
    // themselves. This is shared lens behavior, independent of which control
    // supplies geometry or its animation clock. Keep raw spring overshoot.
    const auto q=std::max(0.f,progress), bounded=std::clamp(q,0.f,1.f);
    auto& p=profile.glass;
    p.bleedDistance0*=q;p.blurDistance0*=q;p.faceBlack*=q;p.faceOpacity*=q;
    p.innerRefractionAmount*=q;p.innerRefractionHeight*=q;p.refractionDistance0*=q;
    p.keyFillHighlightAmount=.4f+(p.keyFillHighlightAmount-.4f)*q;
    p.keyFillHighlightEffectOffset*=q;p.keyFillHighlightHeight*=q;
    constexpr float restingSpread=5*std::numbers::pi_v<float>/12;
    p.keyFillHighlightSpread=restingSpread+(p.keyFillHighlightSpread-restingSpread)*q;
    p.keyFillHighlightSpreadSDR=restingSpread+(p.keyFillHighlightSpreadSDR-restingSpread)*q;
    p.maxHeadroom=1.2f+(p.maxHeadroom-1.2f)*q;
    p.ringShadowBlurRadius=std::min(3.f,1+2*q);p.ringShadowMask=1-q;
    p.ringShadowOffset*=q;p.ringShadowOpacity*=q;p.ringShadowStrokeWidth*=q;
    p.shadowAmount*=q;p.shadowRadius*=q;p.shadowColorMatrixFillColor.a*=bounded;
    p.shadowOffset.y=8-bounded;
    if(!controlLifted)p.shadowOpacity=0;
  }
  // Preserve the finite Gaussian/ring support instead of clipping at an SDF
  // iso-distance. These are kernel bounds, independent of device layout.
  profile.glass.output.maximum=std::max({profile.glass.output.maximum,
      std::abs(profile.glass.ringShadowOffset)+profile.glass.ringShadowStrokeWidth+3*profile.glass.ringShadowBlurRadius,
      std::abs(profile.glass.shadowOffset.y)+3*profile.glass.shadowRadius});
  auto effect=makeNativeGlassEffect(profile.glass,UIGlassEffect::Style::clear,UIColor::clear,false,
      std::abs(profile.backdrop.amount)+std::abs(profile.source.amount)+profile.glass.innerRefractionHeight+32,source);
  if(profile.glass.blurRadius==0 && profile.glass.blurFillRadius==0 &&
     profile.glass.bleedBlurRadius==0 && profile.glass.shadowBlurRadius==0)
    effect.setBackdropBlurRadius(0); // No redundant LOD0 atlas for a sharp lens.
  effect.setBackdropScale(1);
  effect.setUniform("lensProgress",std::clamp(progress,NXFloat(0),NXFloat(1)));
  effect.setUniform("lensControlRefraction",std::array<NXFloat,3>{profile.glass.innerRefractionAmount,profile.glass.innerRefractionHeight,controlReferenceSize?1.f:0.f});
  effect.setUniform("lensBackdropDisplacement",NXPoint(warpsContentBelow?profile.backdrop.amount:0,profile.backdrop.height));
  const auto unitProfile=nativeLensProfile(selection,dark,{1,1});
  effect.setUniform("lensInnerRefractionFraction",NXPoint(unitProfile.glass.innerRefractionAmount,unitProfile.glass.innerRefractionHeight));
  const auto& foreground=unitProfile.foreground;
  effect.setUniform("lensForegroundAberration",std::array<NXFloat,4>{foreground.aberrationAmount,foreground.aberrationHeight,foreground.aberrationOffset,foreground.aberrationAngle});
  effect.setUniform("lensForegroundEdge",std::array<NXFloat,4>{foreground.edgeStart,foreground.edgeEnd,foreground.edgeOpacityStart,foreground.edgeOpacityEnd});
  effect.setUniform("lensForegroundCaptureMargin",foreground.captureMargin);
  return effect;
}
} // namespace NXKit::Internal
