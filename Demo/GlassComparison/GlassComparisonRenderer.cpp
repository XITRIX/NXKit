#include "Scene.h"
#include <BackdropEffect.h>
#include <UIGlassEffect.h>
#include <UITraitCollection.h>
#include "LayerCornerGeometry.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <iostream>
using namespace NXKit;
namespace NXKit {
class GlassMaterialTestHarness {
public:
    static BackdropEffect resolve(UIGlassEffect::Style style, NXSize size, bool dark,
                                 const std::optional<UIColor>& tint) {
        auto traits = UITraitCollection::current()->copy();
        traits->_userInterfaceStyle = dark ? UIUserInterfaceStyle::dark : UIUserInterfaceStyle::light;
        UIGlassEffect effect(style);
        if (tint) effect.setTintColor(*tint);
        return effect._backdropEffect(size, traits);
    }
};
}
void glass(SkCanvas* canvas, NXRect frame, float radius, UIGlassEffect::Style style,
           bool dark, const std::optional<UIColor>& tint) {
    auto effect = GlassMaterialTestHarness::resolve(style, frame.size, dark, tint);
    auto result = SkRuntimeEffect::MakeForShader(SkString(effect.shaderSource()));
    if (!result.effect) { std::cerr << result.errorText.c_str(); std::exit(1); }
    SkRuntimeEffectBuilder builder(result.effect);
    for (const auto* name : {"refraction", "curve", "dispersion", "saturation", "contrast", "luminosity",
         "tint", "edge", "specStrength", "specFallbackStrength", "specBrightFallbackStrength",
         "specLightDirection", "specDirectionalPower", "specWidthPx", "reflectionSamplePx", "reflectionSpreadPx",
         "brightBackdropShade", "referenceAppearance", "bodyMode", "backdropLuma", "edgeAbsorption", "adaptiveTint"}) {
        auto value=effect.uniform(name);
        if (value) builder.uniform(name).set(value->data(), value->size());
    }
    builder.uniform("resolution") = SkV2{frame.width(), frame.height()};
    builder.uniform("lensSize") = SkV2{frame.width(), frame.height()};
    builder.uniform("lensCenter") = SkV2{frame.width()/2, frame.height()/2};
    builder.uniform("cornerRadius") = radius;
    builder.uniform("cornerCurve") = 1.0f;
    Internal::bindLayerCornerGeometry(builder,*result.effect,frame.size,radius);
    builder.uniform("externalShapeCoverage") = 1.0f;
    builder.uniform("contentScale") = 2.0f;
    builder.uniform("maximumSampleRadius") = effect.maximumSampleRadius();
    builder.uniform("interactionIntensity") = 0.0f;
    builder.uniform("interactionLocation") = SkV2{0,0};
    float scale=result.effect->findUniform("filterScale") ? 2.0f : 1.0f;
    if (result.effect->findUniform("filterScale")) builder.uniform("filterScale") = scale;
    auto blur=SkImageFilters::Blur(effect.backdropBlurRadius()*scale,effect.backdropBlurRadius()*scale,SkTileMode::kClamp,nullptr);
    sk_sp<SkImageFilter> filter;
    if(result.effect->findChild("sharpContent")) {
        std::string_view names[]={"content","sharpContent"};
        const sk_sp<SkImageFilter> inputs[]={blur,nullptr};
        filter=SkImageFilters::RuntimeShader(builder,effect.maximumSampleRadius()*scale,names,inputs,2);
    } else filter=SkImageFilters::RuntimeShader(builder,effect.maximumSampleRadius(),"content",blur);
    canvas->save();
    canvas->translate(frame.minX(),frame.minY());
    canvas->clipPath(Internal::layerCornerPath(frame.size, radius, CALayerCornerCurve::continuous), true);
    canvas->scale(1/scale,1/scale);
    auto rect=SkRect::MakeWH(frame.width()*scale,frame.height()*scale);
    SkPaint paint;paint.setImageFilter(filter);
    canvas->saveLayer(SkCanvas::SaveLayerRec(&rect,&paint,SkCanvas::kInitWithPrevious_SaveLayerFlag));
    canvas->restore();canvas->restore();
}
int main(int argc,char**argv) {
    if (argc < 3) { std::cerr << "Usage: GlassComparisonRenderer scene.rgb output.ppm [--dark] [--size=0..1] [--tint=blue|coral]\n"; return 1; }
    bool dark = false;
    float size = 1;
    std::optional<UIColor> tint;
    for (int i = 3; i < argc; ++i) {
        std::string_view argument(argv[i]);
        if (argument == "--dark") dark = true;
        else if (argument == "--tint=blue") tint = UIColor(0, 122, 255);
        else if (argument == "--tint=coral") tint = UIColor(255, 69, 58);
        else if (argument.starts_with("--size=")) {
            size = std::stof(std::string(argument.substr(7)));
            if (!std::isfinite(size) || size < 0 || size > 1) return 1;
        } else { std::cerr << "Unknown comparison argument: " << argument << '\n'; return 1; }
    }
    std::ifstream source(argv[1],std::ios::binary);
    std::vector<unsigned char> data(720*440*3);source.read((char*)data.data(),data.size());
    if (source.gcount() != static_cast<std::streamsize>(data.size())) { std::cerr << "Expected 720 x 440 RGB bytes\n"; return 1; }
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1440,880));
    auto canvas=surface->getCanvas();canvas->scale(2,2);
    SkPaint scenePaint;
    for(int y=0;y<440;++y)for(int x=0;x<720;++x) {
        const auto i=(y*720+x)*3;
        scenePaint.setColor(SkColorSetRGB(data[i],data[i+1],data[i+2]));
        canvas->drawRect(SkRect::MakeXYWH(x,y,1,1),scenePaint);
    }
    for (unsigned index = 0; index < sizeof(GlassSceneElements) / sizeof(GlassSceneElements[0]); ++index) {
        auto element = GlassElementAtSize(index, size);
        glass(canvas, {element.x, element.y, element.width, element.height}, element.radius,
              element.clear ? UIGlassEffect::Style::clear : UIGlassEffect::Style::regular, dark, tint);
    }
    SkBitmap pixels;pixels.allocN32Pixels(1440,880);surface->readPixels(pixels,0,0);
    std::ofstream out(argv[2],std::ios::binary);
    out<<"P6\n1440 880\n255\n";
    for(int y=0;y<880;++y)for(int x=0;x<1440;++x) {
        auto c=pixels.getColor(x,y);out.put(SkColorGetR(c));out.put(SkColorGetG(c));out.put(SkColorGetB(c));
    }
}
