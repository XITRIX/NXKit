#include "LensMaterial.hpp"
#include "GlassVibrancy.hpp"
#include "GlassForeground.hpp"
#include "include/effects/SkImageFilters.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkRuntimeEffect.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPicture.h"
#include "LayerCornerGeometry.hpp"

namespace NXKit::Internal {
void LensLayer::setSource(const std::shared_ptr<UIView>& source, const std::shared_ptr<UIView>& owner) {
  _source = source; _owner = owner;
  if (auto active = presentation()) {
    const auto lens = std::static_pointer_cast<LensLayer>(active);
    lens->_source = source; lens->_owner = owner;
  }
  setNeedsDisplay();
}
void LensLayer::drawSource(SkCanvas* canvas) {
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
    paint.setShader(picture->makeShader(SkTileMode::kDecal,SkTileMode::kDecal,SkFilterMode::kLinear,&pictureToOwner,nullptr));
    paint.setBlender(glassForegroundBlender(_dark));
    paint.setAlphaf(std::clamp(ancestorAlpha,0.0f,1.0f));
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
    auto surface=canvas->makeSurface(SkImageInfo::MakeN32Premul(deviceRect.width(),deviceRect.height(),canvas->imageInfo().refColorSpace()));
    if(surface) {
      auto input=surface->getCanvas();input->clear(SK_ColorTRANSPARENT);
      input->translate(-deviceRect.left(),-deviceRect.top());input->concat(ownerToDevice);
      input->drawPicture(picture,&pictureToOwner,nullptr);
      auto image=surface->makeImageSnapshot();if(image) {
        auto imageToOwner=deviceToOwner;imageToOwner.preTranslate(deviceRect.left(),deviceRect.top());imageToOwner.postScale(sourceScale,sourceScale);
        shader=image->makeShader(SkTileMode::kDecal,SkTileMode::kDecal,SkSamplingOptions(SkFilterMode::kLinear),&imageToOwner);
      }
    }
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
    auto compiled = SkRuntimeEffect::MakeForShader(SkString(std::string(variant?"const bool vibrantSource=true;\n":"const bool vibrantSource=false;\n") + "uniform float cornerCurve;\n" + layerCornerShaderUniforms + layerCornerShaderSource() + glassVibrancySource + R"sksl(
uniform shader selected;
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
// Effective radial fit to the native 1pt line sweep, in logical points.
// These reconstruction parameters are not Apple's private shader equation.
float2 sourcePosition(float2 p) {
  float depth=max(-capsule(p),0.0);
  float projected=sqrt(7.94*7.94+(depth-3.42)*(depth-3.42));
  float shoulder=smoothstep(4.34,13.70,depth);
  float target=projected;
  if(selection<.5) target+=5.3*exp(-depth/1.5)*(1.0-smoothstep(3.0,5.0,depth));
  // Bound the perimeter fold before blending back to identity. The continuous
  // shape's first-order distance can exceed half-height in its interior;
  // clamping the final mixed target used to reintroduce a center displacement.
  target=mix(min(target,min(size.x,size.y)*.5),depth,shoulder);
  return magnifiedPosition(p-opticalNormal(p)*(target-depth));
}
half4 sourceAt(float2 p,float2 normal,float sigma) {
  sigma/=sourceMagnification;
  half4 value=selected.eval(p*sourceScale);
  if(sigma>.05) {
    value*=.375;
    value+=(selected.eval((p-normal*sigma)*sourceScale)+selected.eval((p+normal*sigma)*sourceScale))*.25;
    value+=(selected.eval((p-normal*sigma*2.0)*sourceScale)+selected.eval((p+normal*sigma*2.0)*sourceScale))*.0625;
  }
  return value;
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
    float depth=max(-capsule(p),0.0);
    float2 normal=opticalNormal(p);
    float2 uv=sourcePosition(p);
    float sigma=.6*exp(-depth/1.2);
    half4 green=sourceAt(uv,normal,sigma);
    if(vibrantSource)green=vibrantSample(green,background);
    half3 color=green.rgb;
    half3 alpha=green.aaa;
    if(selection>.5) {
      float edge=1.0-smoothstep(6.0,12.0,depth);
      float v=(3.5-depth)/sqrt((3.5-depth)*(3.5-depth)+2.25);
      // Native selected-ink G/B centroids separate about 1.6pt at the top
      // and 1.05pt at the curved side. The old smaller field left saturated
      // tinted glyphs with narrow fringes despite a strongly dispersed backdrop.
      // Keep the independently measured diagonal fold while increasing the
      // dispersion in the measured top/side sectors. A uniform gain changes
      // the corner's outer/inner branch ownership and loses native ink.
      float side=abs(normal.x);
      float2 spread=float2(1.45+.27*(1.0-smoothstep(.05,.25,side)),
                           .80+.36*smoothstep(.65,.95,side));
      // UIKit's side markers also separate radially (blue inward,
      // red outward). Tangential transport alone leaves long blue glyph
      // strokes with identical channel boundaries along the curved sides.
      float2 shift=(float2(normal.y,normal.x)*spread-normal*(.06*v))*edge;
      float sideRay=smoothstep(.85,.98,side)*edge;
      // Perturb before folding. At curved corners the radial fold can
      // reverse channel order; adding a shift after the fold cannot do so.
      half4 red=sourceAt(sourcePosition(p+shift-normal*(.70*sideRay)),normal,sigma);
      half4 blue=sourceAt(sourcePosition(p-shift+normal*(2.25*sideRay)),normal,sigma);
      if(vibrantSource) { red=vibrantSample(red,background);blue=vibrantSample(blue,background); }
      color=half3(red.r,green.g,blue.b);
      alpha=half3(red.a,green.a,blue.a);
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
  // Native gray controls show a sub-point dark contact contour plus a soft
  // lower lobe. Restrict both to the exterior so the lens body stays clear.
  static const auto effect = [] {
    auto result = SkRuntimeEffect::MakeForShader(SkString(std::string("uniform float cornerCurve;\n") + layerCornerShaderUniforms + layerCornerShaderSource() + R"sksl(
uniform float2 size;
uniform float scale;
uniform float progress;
uniform float selection;
float sdf(float2 p) {
    return boxRoundedSDF(p-size*.5,size*.5,min(size.x,size.y)*.5);
}
half4 main(float2 p) {
    float d=sdf(p);
    if (d<=0.0) return half4(0);
    float2 n=lensNormalDirection(p-size*.5,size*.5,min(size.x,size.y)*.5);
    float aa=.5/max(scale,1.0);
    float contour=(.4+.22*n.x*n.x)*(1.0-smoothstep(1.0-aa,1.0+aa,d));
    float lower=pow(max(n.y,0.0),2.0)*mix(.075,.05,selection)*exp(-pow((d-5.5)/5.0,2.0));
    float ambient=.015*exp(-d/3.0);
    return half4(0,0,0,half(clamp((contour+lower+ambient)*progress,0.0,1.0)));
}
)sksl"));
    if (!result.effect)
      throw std::runtime_error(result.errorText.c_str());
    return result.effect;
  }();
  SkRuntimeShaderBuilder builder(effect);
  builder.uniform("size") = SkV2{bounds().size.width, bounds().size.height};
  builder.uniform("cornerCurve")=1.f;
  bindLayerCornerGeometry(builder,*effect,bounds().size,std::min(bounds().width(),bounds().height())*.5f);
  builder.uniform("scale") = contentsScale();
  builder.uniform("progress") = _progress;
  builder.uniform("selection") = _selection ? 1.f : 0.f;
  SkPaint paint;
  paint.setShader(builder.makeShader());
  canvas->drawRect(SkRect::MakeXYWH(-20, -20, bounds().size.width + 40,
                                    bounds().size.height + 40),
                   paint);
}
BackdropEffect lensMaterial(bool selection, NXFloat progress, bool dark) {
  // Native lifted lenses retain sharp interior detail. Their curved perimeter
  // bends lines and separates colors; there is no regular-glass body veil.
  // This is an independent optical model, not Apple's private shader.
  static const std::string source = std::string("uniform float cornerCurve;\n") + layerCornerShaderUniforms + layerCornerShaderSource() + R"sksl(
uniform shader content;
uniform float2 resolution;
uniform float filterScale;
uniform float contentScale;
uniform float externalShapeCoverage;
uniform float liftProgress;
uniform float refractionStrength;
uniform float chromaticSpread;
uniform float bevelWidth;
uniform float riseWidth;
uniform float ambientLift;
uniform float selectionStyle;
uniform float reflectionStrength;
uniform float reflectionDistance;
uniform float rimShadeScale;

float distanceToCapsule(float2 p) {
    return boxRoundedSDF(p-resolution*.5,resolution*.5,min(resolution.x,resolution.y)*.5);
}
half4 sampleAt(float2 p) { return content.eval(p * filterScale); }
// Native monochrome backdrop probes show tangential RGB separation with a
// finite footprint. This is texture transport, separate from spectral rim
// lighting and from the selected-source fold above.
half4 transported(float2 bent,float2 reflected,float reflection,float2 offset) {
    half4 value=sampleAt(bent+offset);
    if(reflection>0.0) value=mix(value,sampleAt(reflected+offset),half(reflection));
    return value;
}
half4 footprint(float2 bent,float2 reflected,float reflection,float2 offset,float2 tangent,float sigma) {
    half4 value=transported(bent,reflected,reflection,offset)*.375;
    value+=(transported(bent,reflected,reflection,offset+tangent*sigma)+transported(bent,reflected,reflection,offset-tangent*sigma))*.25;
    value+=(transported(bent,reflected,reflection,offset+tangent*sigma*2.0)+transported(bent,reflected,reflection,offset-tangent*sigma*2.0))*.0625;
    return value;
}
half4 main(float2 coordinate) {
    float2 p = coordinate / filterScale;
    half4 original = sampleAt(p);
    float d = distanceToCapsule(p);
    float2 normal = lensNormalDirection(p-resolution*.5,resolution*.5,min(resolution.x,resolution.y)*.5);
    float depth = max(-d, 0.0);
    float bevel = min(bevelWidth, min(resolution.x, resolution.y) * .5);
    float t = clamp(depth / max(bevel,.01),0.0,1.0);
    float displacement = (1.0-exp(-depth/max(riseWidth,.001)))
        * (1.0-t)*(1.0-t) * (1.0+2.0*t) * refractionStrength;
    float2 bent = p + normal * displacement;
    float reflection=reflectionStrength*pow(clamp(1.0-depth/max(reflectionDistance,.001),0.0,1.0),2.0);
    float2 reflected=p-normal*(reflectionDistance-2.0*depth);
    half4 refracted=transported(bent,reflected,reflection,float2(0));
    if(chromaticSpread>0.0 && depth<10.0) {
        // Opposite edges reverse RGB ordering. A radial-only offset cannot
        // reproduce the lateral fringes in the native black/white grid.
        float edge=clamp(1.0-depth/10.0,0.0,1.0);
        float2 shift=float2(normal.y,normal.x*(1.8/3.1))*chromaticSpread*edge;
        float2 tangent=float2(-normal.y,normal.x);
        float sigma=1.2*(1.0-smoothstep(3.0,10.0,depth));
        refracted.r=footprint(bent,reflected,reflection,shift,tangent,sigma).r;
        refracted.g=footprint(bent,reflected,reflection,float2(0),tangent,sigma).g;
        refracted.b=footprint(bent,reflected,reflection,-shift,tangent,sigma).b;
    }
    // Native small-variant uniform controls add the same achromatic lift to
    // all channels: .1 * (1 - luma), including colored backdrops.
    float luma = dot(float3(refracted.rgb),float3(.2126,.7152,.0722));
    refracted.rgb = min(refracted.rgb + half(ambientLift*(1.0-luma)),refracted.aaa);
    float3 shaded;
    if (selectionStyle > .5) {
        // Directional spectral edge observed even over a uniform neutral
        // backdrop. It is independent of RGB texture-coordinate separation.
        float horizontal = normal.x * normal.x;
        float top = max(-normal.y,0.0);
        float bottom = max(normal.y,0.0);
        float sideShade = .05 * horizontal * exp(-pow(max(depth-4.0,0.0)/4.0,2.0));
        float upperShade = .055 * top * exp(-pow((depth-10.0)/6.0,2.0));
        float lowerLight = .03 * bottom * exp(-pow((depth-2.5)/2.0,2.0));
        float upperLight = .015 * top * exp(-pow((depth-2.0)/1.5,2.0));
        float spectral = 1.0-smoothstep(.2,1.75,depth);
        float3 spectralAbsorption = float3(0.0,.045,.42) * horizontal * spectral;
        float3 spectralLight = float3(-.045+.04*normal.y,.18,.25+.05*normal.y)
            * normal.y * normal.y * exp(-depth/.7);
        shaded = float3(refracted.rgb) * (1.0-sideShade-upperShade) * (1.0-spectralAbsorption)
            + lowerLight + upperLight + spectralLight;
    } else {
        float horizontal = normal.x * normal.x;
        float top = max(-normal.y,0.0);
        float bottom = max(normal.y,0.0);
        float sideShade = .09 * horizontal * exp(-pow(max(depth-2.0,0.0)/5.0,2.0));
        float upperShade = top * (.093 * exp(-pow((depth-10.0)/6.0,2.0))
            + .022 * exp(-pow((depth-2.5)/2.0,2.0)));
        float upperLight = .08 * top * exp(-pow((depth-.35)/.35,2.0));
        float lowerLight = bottom * (.24*exp(-depth/.7)
            + .012 * exp(-pow((depth-2.5)/3.0,2.0)));
        shaded = float3(refracted.rgb) * (1.0-rimShadeScale*(sideShade+upperShade)) + lowerLight + upperLight;
    }
    refracted.rgb = half3(min(clamp(shaded,0.0,1.0),float3(refracted.aaa)));
    float coverage = externalShapeCoverage > .5 ? 1.0 : clamp(.5-d*contentScale,0.0,1.0);
    return mix(original,refracted,half(liftProgress)) * half(coverage);
}
)sksl";
  BackdropEffect effect(source, "content", 18, 0);
  effect.setUniform("liftProgress",
                    std::clamp(progress, NXFloat(0), NXFloat(1)));
  effect.setUniform("refractionStrength", selection ? NXFloat(4.5) : NXFloat(4));
  effect.setUniform("chromaticSpread", selection ? NXFloat(3.1) : NXFloat(0));
  effect.setUniform("bevelWidth", NXFloat(28));
  effect.setUniform("riseWidth", selection ? NXFloat(.001) : NXFloat(1));
  effect.setUniform("ambientLift", selection ? NXFloat(0) : NXFloat(.1));
  effect.setUniform("selectionStyle", selection ? NXFloat(1) : NXFloat(0));
  effect.setUniform("reflectionStrength",
                    selection ? NXFloat(.25) : NXFloat(1));
  effect.setUniform("reflectionDistance", selection ? NXFloat(8) : NXFloat(12));
  effect.setUniform("rimShadeScale",
                    !selection && dark ? NXFloat(2.2) : NXFloat(1));
  return effect;
}
} // namespace NXKit::Internal
