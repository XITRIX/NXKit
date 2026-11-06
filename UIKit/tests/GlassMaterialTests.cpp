#include "LayerCornerGeometry.hpp"
#include <BackdropEffect.h>
#include <UIGlassEffect.h>
#include <UITraitCollection.h>
#include "GlassInteraction.hpp"
#include "GlassAppearance.hpp"
#include "NativeGlassProfile.hpp"
#include "NativeGlassShader.hpp"
#include "NativeGlassBlur.hpp"
#include "BackdropShader.hpp"
#include "CABackdropEffectLayer.hpp"
#include "GlassPressCoverageFixture.hpp"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkM44.h"
#include "include/core/SkRRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/effects/SkImageFilters.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <string_view>

using namespace NXKit;

namespace NXKit {
class GlassMaterialTestHarness {
public:
    static BackdropEffect resolve(UIGlassEffect::Style style, NXSize size) {
        return resolve(UIGlassEffect(style),size,false);
    }
    static BackdropEffect resolve(const UIGlassEffect& effect, NXSize size, bool dark, NXFloat amount=.5f, int scale=1) {
        auto traits=UITraitCollection::traitCollectionWithGlassTintAmount(amount);
        traits->_displayScale=scale;
        traits->_userInterfaceStyle=dark ? UIUserInterfaceStyle::dark : UIUserInterfaceStyle::light;
        return effect._backdropEffect(size,traits);
    }
};
}

namespace {
int failures = 0;
void expect(bool result, const char* message) {
    if (result) return;
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}

void testMotion() {
    UIGlassEffect effect;
    expect(!effect.adaptsAppearanceToBackdrop(), "iOS 26 appearance adaptation is disabled by default");
    effect.setAdaptsAppearanceToBackdrop(true);
    auto appearanceCopy = effect;
    effect.setAdaptsAppearanceToBackdrop(false);
    expect(appearanceCopy.adaptsAppearanceToBackdrop() && !effect.adaptsAppearanceToBackdrop(),
           "appearance adaptation has independent copied value semantics");
    expect(!effect.isInteractive(), "UIKit glass is noninteractive by default");
    effect.setInteractive(true);
    auto copy = effect;
    effect.setInteractive(false);
    expect(copy.isInteractive() && !effect.isInteractive(), "interaction is copied with the effect value");
    bool rejected = false;
    try { effect.setEdgeAbsorption(std::numeric_limits<float>::quiet_NaN()); }
    catch (const std::invalid_argument&) { rejected = true; }
    expect(rejected, "meniscus absorption rejects nonfinite values");

    Internal::GlassInteraction sixty, oneTwenty;
    sixty.pressed = oneTwenty.pressed = true;
    sixty.move({20, 20}, {80, 30});
    oneTwenty.move({20, 20}, {80, 30});
    for (int i = 0; i < 60; ++i) sixty.advance(1.0 / 60);
    for (int i = 0; i < 120; ++i) oneTwenty.advance(1.0 / 120);
    expect(std::abs(sixty.inflation.value - oneTwenty.inflation.value) < 0.0001,
           "press motion is independent of refresh rate");
    expect(std::abs(sixty.dragX.value - oneTwenty.dragX.value) < 0.0001,
           "drag response is independent of refresh rate");
    const auto circle = sixty.transform({56, 56});
    Internal::GlassInteraction held;
    held.pressed = true;
    held.advance(2);
    const auto pill = held.transform({132, 56});
    expect(std::abs(pill.m11 * 132 - (132 + 16 - 12 * 12.0 / 116)) < 0.01,
           "large-mode pill inflation resolves from its shortest dimension");
    expect(std::abs(held.transform({56, 56}).m11 * 56 - 72) < 0.01,
           "small circle press grows by sixteen points");
    expect(circle.m11 > 1 && circle.m22 > 1, "a dragged pressed material remains inflated");
    expect(sixty.transform({0, 0}).isIdentity(), "empty glass has an identity visual transform");
    expect(sixty.transform({std::numeric_limits<float>::infinity(), 56}).isIdentity(),
           "nonfinite geometry cannot produce a nonfinite visual transform");

    sixty.pressed = false;
    sixty.advance(0.060);
    expect(sixty.glow() == 0, "press glow is gone within 60 ms of release");
    bool undershot = false;
    for (int i = 0; i < 240; ++i) {
        sixty.advance(1.0 / 120);
        undershot = undershot || sixty.inflation.value < 0;
    }
    expect(undershot, "release has the reference's spring undershoot");
    expect(sixty.atRest() && sixty.transform({132, 56}).isIdentity(),
           "release settles exactly to the application's unmodified geometry");
    sixty.pressed = true;
    sixty.advance(0.05);
    const auto before = sixty.inflation.value;
    sixty.pressed = false;
    expect(sixty.inflation.value == before, "interrupted presses retain their current presentation");
    sixty.advance(10);
    expect(sixty.atRest(), "a long suspended frame safely settles motion");
    sixty.move({std::numeric_limits<float>::infinity(), 0}, {0, 0});
    expect(std::isfinite(sixty.location.x), "nonfinite touch coordinates are ignored");
}

void testAppearancePolicy() {
    using Style = UIUserInterfaceStyle;
    for (auto root : {Style::light, Style::dark}) {
        expect(Internal::glassAppearance({120, 64}, root, 0, root) == Style::dark,
               "small legacy glass chooses dark appearance over black in either app theme");
        expect(Internal::glassAppearance({520, 56}, root, 1, root) == Style::light,
               "a wide short bar chooses light appearance over white in either app theme");
        expect(Internal::glassAppearance({120, 64}, root, 0.5f, root) == root,
               "mid-contrast content retains the app theme");
        for (float backdrop : {0.0f, 1.0f}) {
            expect(Internal::glassAppearance({300, 160}, root, backdrop, Style::dark) == root,
                   "large glass follows the app theme independently of backdrop and history");
        }
        expect(Internal::glassAppearance({0, 64}, root, 0, Style::dark) == root,
               "empty glass cannot choose an adaptive appearance");
        expect(Internal::glassAppearance({120, 64}, root,
                   std::numeric_limits<float>::quiet_NaN(), Style::dark) == root,
               "failed or nonfinite samples retain inherited appearance");
    }
    expect(Internal::glassAppearance({120, 64}, Style::light, 0.25f, Style::dark) == Style::dark
               && Internal::glassAppearance({120, 64}, Style::light, 0.25f, Style::light) == Style::light,
           "the contrast dead band avoids rapid appearance flicker");
}

sk_sp<SkRuntimeEffect> runtime;

void bindMaterial(SkRuntimeEffectBuilder& builder, const BackdropEffect& glass,
                  NXSize size, NXFloat radius, NXFloat scale) {
    // Enumerate the compiled contract so new native passes cannot silently
    // inherit zero uniforms in the test renderer. Automatic host geometry is
    // supplied below, exactly as it is by BackdropShader::makeFilter.
    for (const auto& uniform : runtime->uniforms()) {
        const std::string name(uniform.name);
        const auto components = glass.uniform(name);
        if (components) builder.uniform(name.c_str()).set(components->data(),components->size());
    }
    const auto scalar = [&](const char* name, NXFloat value) {
        if (runtime->findUniform(name)) builder.uniform(name)=value;
    };
    const auto vector = [&](const char* name, SkV2 value) {
        if (runtime->findUniform(name)) builder.uniform(name)=value;
    };
    vector("resolution",{size.width,size.height});
    vector("lensSize",{size.width,size.height});
    vector("lensCenter",{size.width*.5f,size.height*.5f});
    scalar("cornerRadius",radius);scalar("cornerCurve",1);
    Internal::bindLayerCornerGeometry(builder,*runtime,size,radius);
    scalar("contentScale",scale);scalar("filterScale",1);
    scalar("maximumSampleRadius",glass.maximumSampleRadius());
    scalar("externalShapeCoverage",0);scalar("interactionIntensity",0);
    vector("interactionLocation",{0,0});
}

void bindFlatChildren(SkRuntimeEffectBuilder& builder,SkColor background) {
    builder.uniform("backdropPyramidRect")=SkV4{0,0,1,1};
    builder.uniform("backdropPyramidInfo")=SkV4{1,1,1,0};
    // All lanes see the same constant scene for color-transfer controls.
    // Image/filter tests below exercise their different kernels separately.
    for (const auto& child : runtime->children()) {
        const std::string name(child.name);
        builder.child(name.c_str())=SkShaders::Color(background);
    }
}

SkBitmap renderFilterFixture(const UIGlassEffect& material,int scale) {
    constexpr int padding=48,width=120,height=60;
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(
        (width+padding*2)*scale,(height+padding*2)*scale));
    auto* canvas=surface->getCanvas();
    canvas->scale(scale,scale);canvas->clear(SK_ColorBLUE);
    SkPaint stripe;stripe.setColor(SK_ColorRED);
    for (int x=0;x<width+padding*2;x+=8)
        canvas->drawRect(SkRect::MakeXYWH(x,0,4,height+padding*2),stripe);
    const auto glass=GlassMaterialTestHarness::resolve(material,{width,height},false);
    auto filter=Internal::BackdropShader::makeFilter(glass,runtime,{width,height},height*.5f,
        CALayerCornerCurve::continuous,scale);
    expect(filter!=nullptr,"the production six-input image-filter graph is usable on raster");
    canvas->save();canvas->translate(padding,padding);canvas->scale(1.f/scale,1.f/scale);
    const auto rect=SkRect::MakeWH(width*scale,height*scale);
    canvas->clipRRect(SkRRect::MakeRectXY(rect,height*scale*.5f,height*scale*.5f),true);
    SkPaint paint;paint.setImageFilter(filter);
    canvas->saveLayer(SkCanvas::SaveLayerRec(&rect,&paint,SkCanvas::kInitWithPrevious_SaveLayerFlag));
    canvas->restore();canvas->restore();
    SkBitmap pixels;pixels.allocN32Pixels(surface->width(),surface->height());
    expect(surface->readPixels(pixels,0,0),"the actual image-filter graph produces readable pixels");
    return pixels;
}

SkColor render(bool clear,bool dark,NXFloat glow,NXPoint touch,NXPoint sample,
               const UIColor& tint,NXFloat radius=28,NXFloat scale=1,
               SkColor background=SkColorSetRGB(70,100,125)) {
    UIGlassEffect material(clear?UIGlassEffect::Style::clear:UIGlassEffect::Style::regular);
    if (tint.a()>0) material.setTintColor(tint);
    const auto glass=GlassMaterialTestHarness::resolve(material,{132,56},dark);
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(132,56));
    SkRuntimeEffectBuilder builder(runtime);
    bindMaterial(builder,glass,{132,56},radius,scale);
    builder.uniform("interactionIntensity")=glow;
    builder.uniform("interactionLocation")=SkV2{touch.x,touch.y};
    bindFlatChildren(builder,background);
    SkPaint paint;paint.setShader(builder.makeShader());surface->getCanvas()->drawPaint(paint);
    SkBitmap pixels;pixels.allocN32Pixels(132,56);
    expect(surface->readPixels(pixels,0,0),"shader output can be read on the raster backend");
    return pixels.getColor(sample.x,sample.y);
}

int luma(SkColor color) {
    return SkColorGetR(color) * 0.2126 + SkColorGetG(color) * 0.7152 + SkColorGetB(color) * 0.0722;
}

SkColor renderResolvedBody(const BackdropEffect& effect, NXSize size, SkColor background = SK_ColorBLACK, NXPoint sample = {-1, -1}) {
    SkRuntimeEffectBuilder builder(runtime);
    bindMaterial(builder, effect, size, std::min(size.width, size.height) * 0.5f, 1);
    builder.uniform("interactionIntensity") = 0.0f;
    builder.uniform("interactionLocation") = SkV2{0, 0};
    bindFlatChildren(builder,background);
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(size.width, size.height));
    SkPaint paint;
    paint.setShader(builder.makeShader());
    surface->getCanvas()->drawPaint(paint);
    SkBitmap pixels;
    pixels.allocN32Pixels(size.width, size.height);
    surface->readPixels(pixels, 0, 0);
    return sample.x < 0 ? pixels.getColor(size.width / 2, size.height / 2)
        : pixels.getColor(sample.x, sample.y);
}

SkColor renderDefaultBody(UIGlassEffect::Style style, NXSize size, SkColor background = SK_ColorBLACK) {
    return renderResolvedBody(GlassMaterialTestHarness::resolve(style,size),size,background);
}
void testNativeTint() {
    for (bool dark:{false,true}) for (auto style:{UIGlassEffect::Style::clear,UIGlassEffect::Style::regular}) {
        UIGlassEffect glass(style);glass.setTintColor(UIColor(0,122,255));
        const auto resolved=GlassMaterialTestHarness::resolve(glass,{160,70},dark);
        const auto black=renderResolvedBody(resolved,{160,70});
        const int expectedGreen=style==UIGlassEffect::Style::clear ? 71 : (dark ? 122 : 94);
        const int expectedBlue=style==UIGlassEffect::Style::clear ? 166 : (dark ? 255 : 206);
        expect(SkColorGetR(black)<4 && std::abs(static_cast<int>(SkColorGetG(black))-expectedGreen)<6 && std::abs(static_cast<int>(SkColorGetB(black))-expectedBlue)<6,
               "full-alpha tint matches UIKit's clear/regular/light/dark black-backdrop controls");
        const auto white=renderResolvedBody(resolved,{160,70},SK_ColorWHITE);
        expect(SkColorGetB(white)>245 && SkColorGetG(white)>115,"tinted glass retains the supplied color on white");
        glass.setTintColor(UIColor(0,122,255,128));
        const auto alpha=renderResolvedBody(GlassMaterialTestHarness::resolve(glass,{160,70},dark),{160,70});
        expect(SkColorGetB(alpha)<SkColorGetB(black) && SkColorGetA(alpha)==255,"tint alpha interpolates with material without punching out its backdrop");
        glass.setTintOpacity(0);
        const auto override=GlassMaterialTestHarness::resolve(glass,{160,70},dark);
        expect(override.uniform("nativeAccentEnabled")->front()==0 && override.uniform("nativeTintOpacity")->front()==0,"explicit opacity overrides retain the low-level tint path");
    }
}

void testNativeProfiles() {
    using namespace Internal;
    using Style = UIGlassEffect::Style;
    using Appearance = UIUserInterfaceStyle;
    for (auto appearance : {Appearance::light, Appearance::dark}) {
        const auto zero = nativeGlassProfile(Style::regular, appearance, 0);
        const auto middle = nativeGlassProfile(Style::regular, appearance, .5f);
        const auto full = nativeGlassProfile(Style::regular, appearance, 1);
        expect(zero.blurRadius == 5 && middle.blurRadius == 5 && full.blurRadius == 5,
               "captured regular main blur survives every system tint setting");
        expect(zero.blurFillRadius == 8 && zero.blurFillNormalOpacity == 0
               && middle.blurFillNormalOpacity == .5f && full.blurFillNormalOpacity == 1,
               "native main blur and normal fill are independent inputs");
        for (const auto style : {Style::regular,Style::clear}) {
            for (int i=0;i<=100;++i) {
                const auto profile=nativeGlassProfile(style,appearance,i/100.f,{112,64});
                const auto material=makeNativeGlassEffect(profile,style,UIColor::white,false);
                const auto radii=*material.uniform("nativeBlurRadii");
                expect(material.backdropPyramid().has_value() && material.backdropInputs().empty()
                    &&material.backdropScale()==profile.context.backdropScale
                    &&radii[0]==profile.blurRadius
                    &&radii[1]==((profile.blurFillNormalOpacity>0 || profile.blurFillLightenOpacity>0 || profile.blurFillDarkenOpacity>0)?profile.blurFillRadius:0)
                    &&radii[2]==(profile.bleedOpacity>0?profile.bleedBlurRadius*.5f:0)
                    &&radii[3]==((profile.shadowOpacity>0 && profile.shadowVibrancyContribution>0)?profile.shadowBlurRadius*.5f:0),
                       "main, fill, bleed and shadow retain independent native radii in one pyramid");
            }
        }
        const auto baseline=makeNativeGlassEffect(middle,Style::regular,UIColor::white,false);
        auto changed=middle;changed.blurRadius*=2;
        expect(makeNativeGlassEffect(changed,Style::regular,UIColor::white,false).uniform("nativeBlurRadii")->at(0)
                   >baseline.uniform("nativeBlurRadii")->at(0),"native main blur directly controls its own kernel");
        changed=middle;changed.blurFillRadius*=2;
        const auto fill=makeNativeGlassEffect(changed,Style::regular,UIColor::white,false);
        expect(fill.uniform("nativeBlurRadii")->at(1)>baseline.uniform("nativeBlurRadii")->at(1)
                   &&fill.uniform("nativeBlurRadii")->at(0)==baseline.uniform("nativeBlurRadii")->at(0),
               "fill blur changes independently without being collapsed into the main kernel");
        changed=middle;changed.innerRefractionAmount*=.5f;changed.innerRefractionHeight*=.5f;
        const auto lens=makeNativeGlassEffect(changed,Style::regular,UIColor::white,false);
        expect(lens.uniform("nativeRefraction")->at(0)==changed.innerRefractionAmount
                   &&lens.uniform("nativeRefraction")->at(1)==changed.innerRefractionHeight,
               "native displacement and bevel height retain independent point units");
        expect(nativeGlassProfile(Style::clear, appearance, 0).blurRadius == 0,
               "native clear style has no main blur at zero preference");
        for (const auto amount : {.25f,.75f}) {
            const auto profile = nativeGlassProfile(Style::regular, appearance, amount);
            expect(profile.blurFillNormalOpacity == amount,
                   "the documented portable interpolation policy preserves anchor intervals");
        }
    }
    for (auto amount : {-1.f, 2.f, std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()}) {
        bool rejected = false;
        try { (void)nativeGlassProfile(Style::regular, Appearance::dark, amount); }
        catch (const std::invalid_argument&) { rejected = true; }
        expect(rejected, "native-profile resolution rejects invalid preference values");
    }
}

void testGlassTintPreference() {
    for(const bool dark : {false,true}) for(const auto style : {UIGlassEffect::Style::regular,UIGlassEffect::Style::clear}) {
        for(const bool tinted : {false,true}) {
            UIGlassEffect glass(style);
            if(tinted) glass.setTintColor(UIColor(0,122,255));
            int previousTransmission=1000;
            for(const auto amount : {0.f,.25f,.5f,.75f,1.f}) {
                const auto effect=GlassMaterialTestHarness::resolve(glass,{160,64},dark,amount);
                const auto profile=Internal::nativeGlassProfile(style,dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light,
                    amount,{160,64},tinted);
                expect(effect.uniform("nativeFillOpacity")->front()==profile.blurFillNormalOpacity,
                       "the inherited preference resolves the captured native normal-fill lane");
                const auto black=renderResolvedBody(effect,{160,64},SK_ColorBLACK);
                const auto white=renderResolvedBody(effect,{160,64},SK_ColorWHITE);
                const int transmission=std::abs(int(SkColorGetR(white))-int(SkColorGetR(black)))
                    +std::abs(int(SkColorGetG(white))-int(SkColorGetG(black)))
                    +std::abs(int(SkColorGetB(white))-int(SkColorGetB(black)));
                expect(transmission<=previousTransmission+3,"tinting continuously reduces backdrop transmission in both styles/themes, including colored glass");
                previousTransmission=transmission;
                expect(SkColorGetA(black)==255 && SkColorGetA(white)==255,"glass preferences change the body rather than fading the view");
                if(amount==1 && !tinted) expect(transmission>100,"native fully tinted glass still transmits backdrop color");
            }
            glass.setFrostRadius(7);
            expect(GlassMaterialTestHarness::resolve(glass,{160,64},dark,0).backdropBlurRadius()==7,
                   "an explicit frost override retains its value at the clear endpoint");
        }
        UIGlassEffect blue(style),red(style);
        blue.setTintColor(UIColor(0,0,255,0));red.setTintColor(UIColor(255,0,0,0));
        for(const auto amount : {.75f,1.f}) {
            const auto a=renderResolvedBody(GlassMaterialTestHarness::resolve(blue,{160,64},dark,amount),{160,64},SK_ColorWHITE);
            const auto b=renderResolvedBody(GlassMaterialTestHarness::resolve(red,{160,64},dark,amount),{160,64},SK_ColorWHITE);
            expect(a==b,"increasing glass tint cannot reveal the hidden RGB of a fully transparent custom tint");
        }
    }
}

void testNativeGlassTintControls() {
    struct Control { float amount; bool dark,clear,tinted; int width,height,r,g,b,nr,ng,nb; };
    const Control controls[] = {
#include "fixtures/GlassTintReference.inc"
    };
    int maximumError=0;
    double totalError=0;
    for(const auto& c:controls) {
        UIGlassEffect effect(c.clear?UIGlassEffect::Style::clear:UIGlassEffect::Style::regular);
        if(c.tinted) effect.setTintColor(UIColor(0,122,255));
        const NXSize size(c.width,c.height);
        const auto pixel=renderResolvedBody(GlassMaterialTestHarness::resolve(effect,size,c.dark,c.amount),size,SkColorSetRGB(c.r,c.g,c.b));
        const int error=std::max({std::abs(int(SkColorGetR(pixel))-c.nr),std::abs(int(SkColorGetG(pixel))-c.ng),std::abs(int(SkColorGetB(pixel))-c.nb)});
        maximumError=std::max(maximumError,error);
        totalError+=std::abs(int(SkColorGetR(pixel))-c.nr)+std::abs(int(SkColorGetG(pixel))-c.ng)+std::abs(int(SkColorGetB(pixel))-c.nb);
        if(error>16) std::cerr<<"Native glass residual "<<c.amount<<" dark="<<c.dark<<" clear="<<c.clear<<" tinted="<<c.tinted<<" background="<<c.r<<','<<c.g<<','<<c.b<<" error="<<error<<'\n';
        expect(error<=16,"the preference/style/color matrix stays within measured native material tolerance");
    }
    std::cout<<"Native preference controls: "<<std::size(controls)<<", mean channel error "<<totalError/(3*std::size(controls))<<", max "<<maximumError<<'\n';
}

void testNativeOpticalControls() {
    // Independent iPadOS 27.0 sRGB framebuffer controls, captured by
    // Demo/GlassComparison/OpticalReference.m. See docs/liquid-glass-optics.
    // Yellow was withheld from fitting; the larger light neutral residual is
    // retained with its measured tolerance instead of adjusting the reference.
    struct Control { bool dark; int tint; NXFloat size; SkColor background, expected; int tolerance; };
    const Control controls[] = {
        {false,0,52,SkColorSetRGB(0,0,0),SkColorSetRGB(133,133,133),6},
        {false,0,120,SkColorSetRGB(0,0,0),SkColorSetRGB(135,135,135),6},
        {false,0,52,SkColorSetRGB(128,128,128),SkColorSetRGB(195,195,195),6},
        {false,0,120,SkColorSetRGB(128,128,128),SkColorSetRGB(200,200,200),6},
        {false,0,52,SkColorSetRGB(255,255,255),SkColorSetRGB(253,253,253),6},
        {false,0,120,SkColorSetRGB(255,255,255),SkColorSetRGB(254,254,254),6},
        {false,0,52,SkColorSetRGB(255,0,0),SkColorSetRGB(255,108,108),6},
        {false,0,120,SkColorSetRGB(255,0,0),SkColorSetRGB(255,111,111),6},
        {false,0,52,SkColorSetRGB(0,255,0),SkColorSetRGB(51,255,51),6},
        {false,0,120,SkColorSetRGB(0,255,0),SkColorSetRGB(48,255,48),6},
        {false,0,52,SkColorSetRGB(0,0,255),SkColorSetRGB(124,124,255),6},
        {false,0,120,SkColorSetRGB(0,0,255),SkColorSetRGB(127,127,255),6},
        {false,0,52,SkColorSetRGB(255,255,0),SkColorSetRGB(255,255,27),6},
        {false,0,120,SkColorSetRGB(255,255,0),SkColorSetRGB(255,255,14),12},
        {false,1,52,SkColorSetRGB(0,0,0),SkColorSetRGB(0,94,206),4},
        {false,1,120,SkColorSetRGB(0,0,0),SkColorSetRGB(0,95,207),4},
        {false,1,52,SkColorSetRGB(128,128,128),SkColorSetRGB(0,108,231),4},
        {false,1,120,SkColorSetRGB(128,128,128),SkColorSetRGB(0,109,233),4},
        {false,1,52,SkColorSetRGB(255,255,255),SkColorSetRGB(0,122,254),4},
        {false,1,120,SkColorSetRGB(255,255,255),SkColorSetRGB(0,122,255),4},
        {false,1,52,SkColorSetRGB(255,0,0),SkColorSetRGB(0,96,209),4},
        {false,1,120,SkColorSetRGB(255,0,0),SkColorSetRGB(0,96,210),4},
        {false,1,52,SkColorSetRGB(0,255,0),SkColorSetRGB(0,109,232),4},
        {false,1,120,SkColorSetRGB(0,255,0),SkColorSetRGB(0,109,231),4},
        {false,1,52,SkColorSetRGB(0,0,255),SkColorSetRGB(0,94,206),4},
        {false,1,120,SkColorSetRGB(0,0,255),SkColorSetRGB(0,95,207),4},
        {false,1,52,SkColorSetRGB(255,255,0),SkColorSetRGB(0,118,248),4},
        {false,1,120,SkColorSetRGB(255,255,0),SkColorSetRGB(0,118,248),4},
        {false,2,52,SkColorSetRGB(0,0,0),SkColorSetRGB(228,30,25),4},
        {false,2,120,SkColorSetRGB(0,0,0),SkColorSetRGB(228,31,26),4},
        {false,2,52,SkColorSetRGB(128,128,128),SkColorSetRGB(242,50,42),4},
        {false,2,120,SkColorSetRGB(128,128,128),SkColorSetRGB(243,51,43),4},
        {false,2,52,SkColorSetRGB(255,255,255),SkColorSetRGB(255,68,57),4},
        {false,2,120,SkColorSetRGB(255,255,255),SkColorSetRGB(255,69,58),4},
        {false,2,52,SkColorSetRGB(255,0,0),SkColorSetRGB(229,32,27),4},
        {false,2,120,SkColorSetRGB(255,0,0),SkColorSetRGB(230,33,28),4},
        {false,2,52,SkColorSetRGB(0,255,0),SkColorSetRGB(242,51,42),4},
        {false,2,120,SkColorSetRGB(0,255,0),SkColorSetRGB(242,50,42),4},
        {false,2,52,SkColorSetRGB(0,0,255),SkColorSetRGB(228,30,25),4},
        {false,2,120,SkColorSetRGB(0,0,255),SkColorSetRGB(228,31,26),4},
        {false,2,52,SkColorSetRGB(255,255,0),SkColorSetRGB(251,64,54),4},
        {false,2,120,SkColorSetRGB(255,255,0),SkColorSetRGB(251,63,53),4},
        {true,0,52,SkColorSetRGB(0,0,0),SkColorSetRGB(32,32,32),6},
        {true,0,120,SkColorSetRGB(0,0,0),SkColorSetRGB(32,32,32),6},
        {true,0,52,SkColorSetRGB(128,128,128),SkColorSetRGB(134,134,134),6},
        {true,0,120,SkColorSetRGB(128,128,128),SkColorSetRGB(117,117,117),6},
        {true,0,52,SkColorSetRGB(255,255,255),SkColorSetRGB(185,185,185),6},
        {true,0,120,SkColorSetRGB(255,255,255),SkColorSetRGB(121,121,121),6},
        {true,0,52,SkColorSetRGB(255,0,0),SkColorSetRGB(255,15,15),6},
        {true,0,120,SkColorSetRGB(255,0,0),SkColorSetRGB(255,14,14),6},
        {true,0,52,SkColorSetRGB(0,255,0),SkColorSetRGB(0,235,0),6},
        {true,0,120,SkColorSetRGB(0,255,0),SkColorSetRGB(0,187,0),6},
        {true,0,52,SkColorSetRGB(0,0,255),SkColorSetRGB(26,26,255),6},
        {true,0,120,SkColorSetRGB(0,0,255),SkColorSetRGB(25,25,255),6},
        {true,0,52,SkColorSetRGB(255,255,0),SkColorSetRGB(197,197,0),6},
        {true,0,120,SkColorSetRGB(255,255,0),SkColorSetRGB(137,137,0),6},
        {true,1,52,SkColorSetRGB(0,0,0),SkColorSetRGB(0,121,255),4},
        {true,1,120,SkColorSetRGB(0,0,0),SkColorSetRGB(0,121,255),4},
        {true,1,52,SkColorSetRGB(128,128,128),SkColorSetRGB(20,130,251),4},
        {true,1,120,SkColorSetRGB(128,128,128),SkColorSetRGB(16,129,252),4},
        {true,1,52,SkColorSetRGB(255,255,255),SkColorSetRGB(30,135,249),4},
        {true,1,120,SkColorSetRGB(255,255,255),SkColorSetRGB(17,129,252),4},
        {true,1,52,SkColorSetRGB(255,0,0),SkColorSetRGB(6,124,254),4},
        {true,1,120,SkColorSetRGB(255,0,0),SkColorSetRGB(6,124,254),4},
        {true,1,52,SkColorSetRGB(0,255,0),SkColorSetRGB(27,133,250),4},
        {true,1,120,SkColorSetRGB(0,255,0),SkColorSetRGB(19,130,251),4},
        {true,1,52,SkColorSetRGB(0,0,255),SkColorSetRGB(1,122,255),4},
        {true,1,120,SkColorSetRGB(0,0,255),SkColorSetRGB(1,122,255),4},
        {true,1,52,SkColorSetRGB(255,255,0),SkColorSetRGB(30,135,250),4},
        {true,1,120,SkColorSetRGB(255,255,0),SkColorSetRGB(18,130,252),4},
        {true,2,52,SkColorSetRGB(0,0,0),SkColorSetRGB(255,68,57),4},
        {true,2,120,SkColorSetRGB(0,0,0),SkColorSetRGB(255,68,57),4},
        {true,2,52,SkColorSetRGB(128,128,128),SkColorSetRGB(252,90,80),4},
        {true,2,120,SkColorSetRGB(128,128,128),SkColorSetRGB(253,86,76),4},
        {true,2,52,SkColorSetRGB(255,255,255),SkColorSetRGB(251,101,92),4},
        {true,2,120,SkColorSetRGB(255,255,255),SkColorSetRGB(253,87,77),4},
        {true,2,52,SkColorSetRGB(255,0,0),SkColorSetRGB(254,75,64),4},
        {true,2,120,SkColorSetRGB(255,0,0),SkColorSetRGB(254,75,64),4},
        {true,2,52,SkColorSetRGB(0,255,0),SkColorSetRGB(251,97,88),4},
        {true,2,120,SkColorSetRGB(0,255,0),SkColorSetRGB(252,90,80),4},
        {true,2,52,SkColorSetRGB(0,0,255),SkColorSetRGB(255,70,59),4},
        {true,2,120,SkColorSetRGB(0,0,255),SkColorSetRGB(255,70,59),4},
        {true,2,52,SkColorSetRGB(255,255,0),SkColorSetRGB(251,100,91),4},
        {true,2,120,SkColorSetRGB(255,255,0),SkColorSetRGB(253,88,79),4},
        // Independent pastel holdouts: neither tint nor body fitting used these.
        {false,0,52,SkColorSetRGB(180,120,200),SkColorSetRGB(239,183,255),6},
        {false,0,120,SkColorSetRGB(180,120,200),SkColorSetRGB(245,187,255),6},
        {true,0,52,SkColorSetRGB(180,120,200),SkColorSetRGB(185,120,207),6},
        {true,0,120,SkColorSetRGB(180,120,200),SkColorSetRGB(159,103,177),6},
        {false,0,52,SkColorSetRGB(128,200,160),SkColorSetRGB(170,237,200),6},
        {false,0,120,SkColorSetRGB(128,200,160),SkColorSetRGB(172,242,204),6},
        {true,0,52,SkColorSetRGB(128,200,160),SkColorSetRGB(108,180,140),6},
        {true,0,120,SkColorSetRGB(128,200,160),SkColorSetRGB(86,143,111),6},
        {false,0,52,SkColorSetRGB(200,160,128),SkColorSetRGB(245,207,177),6},
        {false,0,120,SkColorSetRGB(200,160,128),SkColorSetRGB(251,212,181),6},
        {true,0,52,SkColorSetRGB(200,160,128),SkColorSetRGB(190,148,115),6},
        {true,0,120,SkColorSetRGB(200,160,128),SkColorSetRGB(155,121,94),6},
    };
    for (const auto& c : controls) {
        UIGlassEffect glass;
        if (c.tint) glass.setTintColor(c.tint == 1 ? UIColor(0,122,255) : UIColor(255,69,58));
        const auto resolved = GlassMaterialTestHarness::resolve(glass, {c.size,c.size}, c.dark);
        const auto actual = renderResolvedBody(resolved, {c.size,c.size}, c.background);
        const int error = std::max({std::abs(int(SkColorGetR(actual))-int(SkColorGetR(c.expected))),
            std::abs(int(SkColorGetG(actual))-int(SkColorGetG(c.expected))),
            std::abs(int(SkColorGetB(actual))-int(SkColorGetB(c.expected)))});
        if (error > c.tolerance) std::cerr << "Native control dark=" << c.dark << " tint=" << c.tint
            << " size=" << c.size << " background=" << std::hex << c.background
            << " actual=" << actual << " expected=" << c.expected << std::dec << '\n';
        expect(error <= c.tolerance, "regular body and tint match native solid-color controls");
    }
    UIGlassEffect glass;
    glass.setLightAngle(-90);
    auto resolved = GlassMaterialTestHarness::resolve(glass,{52,52},false);
    const auto background = SkColorSetRGB(128,128,128);
    const auto top = renderResolvedBody(resolved,{52,52},background,{26,0});
    const auto side = renderResolvedBody(resolved,{52,52},background,{0,26});
    std::cout << "Rim control top=" << luma(top) << " side=" << luma(side) << '\n';
    expect(luma(top) > luma(side) + 12, "regular rim is directed light rather than a white border");
    glass.setLightAngle(0);
    resolved = GlassMaterialTestHarness::resolve(glass,{52,52},false);
    expect(luma(renderResolvedBody(resolved,{52,52},background,{0,26}))
        > luma(renderResolvedBody(resolved,{52,52},background,{26,0})) + 12,
        "the regular light direction follows the optical override");
    glass.setSpecularStrength(0);
    resolved = GlassMaterialTestHarness::resolve(glass,{52,52},false);
    expect(luma(renderResolvedBody(resolved,{52,52},background,{0,26})) < luma(top)-12,
        "zero specular strength removes the regular rim reflection");
    glass.setFrostRadius(0); glass.setSaturation(1); glass.setContrast(1); glass.setTintOpacity(0);
    resolved = GlassMaterialTestHarness::resolve(glass,{52,52},false);
    expect(resolved.backdropBlurRadius() == 0, "explicit zero frost overrides the regular preset");
    expect(renderResolvedBody(resolved,{52,52},SK_ColorRED) == SK_ColorRED,
        "explicit material overrides bypass the fitted default tone");
}

void testBackdropInputs() {
    // A source impulse makes each Gaussian lane independently observable;
    // sharpContent must retain the original pixels at reduced capture scale.
    BackdropEffect effect(R"sksl(
uniform shader content;
uniform shader narrowContent;
uniform shader wideContent;
uniform shader sharpContent;
uniform float filterScale;
uniform float4 lane;
half4 main(float2 xy) {
    return content.eval(xy)*half(lane.x)+narrowContent.eval(xy)*half(lane.y)
        +wideContent.eval(xy)*half(lane.z)+sharpContent.eval(xy)*half(lane.w);
})sksl","content",32,0);
    effect.setBackdropInput("narrowContent",3);
    effect.setBackdropInput("wideContent",7);
    effect.setBackdropScale(.5f);
    effect.setUniform("lane",std::array<NXFloat,4>{0,0,0,1});
    const auto program=Internal::BackdropShader::compile(effect);
    Internal::BackdropShader::validate(effect,*program);
    const auto image = [&](const BackdropEffect& description,int scale) {
        auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(64*scale,32*scale));
        surface->getCanvas()->clear(SK_ColorBLACK);
        SkPaint stripe;stripe.setColor(SK_ColorWHITE);
        surface->getCanvas()->drawRect(SkRect::MakeXYWH(30*scale,0,3*scale,32*scale),stripe);
        auto filter=Internal::BackdropShader::makeFilter(description,program,{64,32},0,
            CALayerCornerCurve::circular,scale);
        expect(filter!=nullptr,"multiple named backdrop lanes produce an image filter");
        const auto rect=SkRect::MakeWH(64*scale,32*scale);
        SkPaint paint;paint.setImageFilter(filter);
        surface->getCanvas()->saveLayer(SkCanvas::SaveLayerRec(&rect,&paint,SkCanvas::kInitWithPrevious_SaveLayerFlag));
        surface->getCanvas()->restore();
        SkBitmap pixels;pixels.allocN32Pixels(64*scale,32*scale);
        expect(surface->readPixels(pixels,0,0),"named backdrop lane pixels are readable");
        return pixels;
    };
    for (int scale:{1,2,3}) {
        const auto sharp=image(effect,scale);
        for (int x=25*scale;x<38*scale;++x)
            expect(SkColorGetR(sharp.getColor(x,16*scale))==(x>=30*scale&&x<33*scale?255:0),
                   "sharpContent bypasses capture resampling at every backing scale");
        auto selected=effect;
        selected.setUniform("lane",std::array<NXFloat,4>{1,0,0,0});
        const auto captured=image(selected,scale);
        int resamplingDifference=0;
        for(int x=25*scale;x<38*scale;++x)
            resamplingDifference+=std::abs(int(SkColorGetR(captured.getColor(x,16*scale)))
                -int(SkColorGetR(sharp.getColor(x,16*scale))));
        expect(resamplingDifference>30,
               "zero-sigma captured input really resamples; reciprocal scale matrices must not cancel");
        selected.setUniform("lane",std::array<NXFloat,4>{0,1,0,0});
        const auto narrow=image(selected,scale).getColor(31*scale,16*scale);
        selected.setUniform("lane",std::array<NXFloat,4>{0,0,1,0});
        const auto wide=image(selected,scale).getColor(31*scale,16*scale);
        expect(SkColorGetR(narrow)>SkColorGetR(wide)+20&&SkColorGetR(narrow)<180,
               "independent Gaussian sigmas remain in logical points through reduced-resolution capture");
        selected.setBackdropInput("wideContent",3);
        const auto equal=image(selected,scale).getColor(31*scale,16*scale);
        expect(std::abs(int(SkColorGetR(equal))-int(SkColorGetR(narrow)))<=1,
               "equal named kernels sample the same original scene without compositing feedback");
        expect(effect.backdropInputs().at("wideContent")==7,
               "editing a copied named-input graph does not change the original description");
    }
    const auto rejected = [&](const auto& mutation,const char* message) {
        bool caught=false;
        try { auto invalid=effect;mutation(invalid); }
        catch (const std::invalid_argument&) { caught=true; }
        expect(caught,message);
    };
    for (const auto* name:{"","bad name","9input","content","sharpContent"})
        rejected([&](auto& value){value.setBackdropInput(name,1);},
                 "named backdrop inputs reject invalid identifiers and reserved primary/sharp lanes");
    for (NXFloat sigma:{-1.f,std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()})
        rejected([&](auto& value){value.setBackdropInput("narrowContent",sigma);},
                 "named Gaussian kernels reject negative and nonfinite radii");
    for (NXFloat scale:{0.f,-1.f,1.1f,std::numeric_limits<float>::quiet_NaN()})
        rejected([&](auto& value){value.setBackdropScale(scale);},
                 "capture resampling rejects nonfinite and out-of-range scales");
    rejected([&](auto& value){value.setBackdropInput("undeclared",1);Internal::BackdropShader::validate(value,*program);},
             "validation rejects named lanes absent from the compiled shader contract");
    BackdropEffect missing(effect.shaderSource());missing.setBackdropInput("narrowContent",3);
    bool caught=false;
    try { Internal::BackdropShader::validate(missing,*program); }
    catch (const std::invalid_argument&) { caught=true; }
    expect(caught,"validation rejects shader children that have no declared input lane");
}

void testNativeFaceHoldouts() {
    struct Control {float size,saturation,black,white;int r,g,b,nr,ng,nb;};
    const Control controls[]={
#include "fixtures/NativeGlassFaceHoldouts.inc"
    };
    for(const auto& c:controls) {
        auto profile=Internal::nativeGlassProfile(UIGlassEffect::Style::regular,UIUserInterfaceStyle::dark,.5f,{c.size,c.size},false);
        profile.faceSaturation=c.saturation;profile.faceBlack=c.black;profile.faceWhite=c.white;
        profile.bleedOpacity=profile.ringShadowOpacity=profile.shadowOpacity=profile.sdrShadowOpacity=profile.shadowAmount=profile.keyFillHighlightAmount=0;
        const auto effect=Internal::makeNativeGlassEffect(profile,UIGlassEffect::Style::regular,UIColor::white,false);
        const auto actual=renderResolvedBody(effect,{c.size,c.size},SkColorSetRGB(c.r,c.g,c.b));
        const auto error=std::max({std::abs(int(SkColorGetR(actual))-c.nr),std::abs(int(SkColorGetG(actual))-c.ng),std::abs(int(SkColorGetB(actual))-c.nb)});
        if(error>2)std::cerr<<"Native face size="<<c.size<<" saturation="<<c.saturation<<" error="<<error<<'\n';
        expect(error<=2,"native face transfer matches independent endpoint and saturation perturbations");
    }
}

void testNativeHoldingComposition() {
    // Native holding-tone captures isolate a multiplicative white pass. This
    // exercises its placement in the complete material, beyond helper tests.
    for(float size:{64.f,120.f}) for(bool enabled:{false,true})
        for(SkColor background:{SkColorSetRGB(80,80,80),SkColorSetRGB(120,70,80),SK_ColorWHITE}) {
            auto p=Internal::nativeGlassProfile(UIGlassEffect::Style::regular,
                UIUserInterfaceStyle::dark,.5f,{size,size},false);
            p.faceBlack=0;p.faceWhite=p.faceSaturation=1;
            p.faceColorMatrixMaxLuma=p.faceColorMatrixMaxLumaSDR=1;
            p.faceColorMatrixFillColor={0,0,0,0};p.faceOpacity=1;
            p.blurRadius=p.blurFillRadius=p.bleedBlurRadius=p.shadowBlurRadius=0;
            p.blurFillNormalOpacity=p.blurFillLightenOpacity=p.blurFillDarkenOpacity=0;
            p.innerRefractionAmount=p.outerRefractionAmount=p.refractionOpacity=0;
            p.bleedOpacity=p.shadowOpacity=p.sdrShadowOpacity=p.ringShadowOpacity=0;
            p.keyFillHighlightAmount=p.highlight.keyAmount=p.highlight.fillAmount=0;
            p.sdrHoldingToneEnabled=enabled;p.sdrHoldingToneWhite=.5f;
            p.sdrGradientDistance0=100;p.sdrGradientDistance1=101;
            const auto effect=Internal::makeNativeGlassEffect(p,UIGlassEffect::Style::regular,UIColor::clear,false);
            const auto actual=renderResolvedBody(effect,{size,size},background);
            const float gain=enabled?.5f:1.f;
            const std::array<float,3> actualRGB={float(SkColorGetR(actual)),float(SkColorGetG(actual)),float(SkColorGetB(actual))};
            const std::array<float,3> backgroundRGB={float(SkColorGetR(background)),float(SkColorGetG(background)),float(SkColorGetB(background))};
            for(size_t i=0;i<3;++i)
                expect(std::abs(actualRGB[i]-gain*backgroundRGB[i])<=1,
                    "holding tone multiplies the complete neutral material before decoration");
        }
}

void testNativeTransparentMaterial() {
    for(float outerOpacity:{0.f,.5f,1.f}) {
        auto p=Internal::nativeGlassProfile(UIGlassEffect::Style::regular,
            UIUserInterfaceStyle::dark,.5f,{64,64},false);
        p.faceBlack=.2f;p.faceWhite=p.faceSaturation=1;
        p.faceColorMatrixMaxLuma=p.faceColorMatrixMaxLumaSDR=1;
        p.faceColorMatrixFillColor={0,0,0,0};p.faceOpacity=1;
        p.blurRadius=p.blurFillRadius=p.bleedBlurRadius=p.shadowBlurRadius=0;
        p.blurFillNormalOpacity=p.blurFillLightenOpacity=p.blurFillDarkenOpacity=0;
        p.innerRefractionAmount=p.outerRefractionAmount=0;
        p.refractionOpacity=outerOpacity;p.refractionDistance0=-100;p.refractionDistance1=-99;
        p.bleedOpacity=p.shadowOpacity=p.sdrShadowOpacity=p.ringShadowOpacity=0;
        p.keyFillHighlightAmount=p.highlight.keyAmount=p.highlight.fillAmount=0;
        p.sdrHoldingToneEnabled=false;
        const auto effect=Internal::makeNativeGlassEffect(p,UIGlassEffect::Style::regular,UIColor::clear,false);
        const auto actual=renderResolvedBody(effect,{64,64},SK_ColorTRANSPARENT);
        const float alpha=1-outerOpacity;
        expect(std::abs(float(SkColorGetA(actual))-255*alpha)<=1,
            "native material coverage is independent of inner backdrop alpha");
        const float red=alpha>0?255*.2f/alpha:0;
        expect(std::abs(float(SkColorGetR(actual))-red)<=2,
            "face preserves literal outer RGBA until final premultiplied clamp");
    }
}

void testNativeBlurRangePacking() {
    auto p=Internal::nativeGlassProfile(UIGlassEffect::Style::regular,UIUserInterfaceStyle::dark,.5f);
    p.blurRadius=0;p.blurOpacity0=p.blurOpacity1=p.blurOpacity2=p.blurOpacity3=1;
    p.blurFillNormalOpacity=p.blurFillLightenOpacity=p.blurFillDarkenOpacity=0;
    p.bleedOpacity=1;p.bleedBlurRadius=20;p.shadowOpacity=1;p.shadowBlurRadius=12;
    p.shadowVibrancyContribution=0;
    auto effect=Internal::makeNativeGlassEffect(p,UIGlassEffect::Style::regular,UIColor::clear,false);
    expect(effect.uniform("nativeMinimumBlurRadius")->front()==0
        &&effect.uniform("nativeBlurRadii")->at(2)==10
        &&effect.uniform("nativeBlurRadii")->at(3)==0,
        "native range keeps zero main radius and halves active bleed while excluding nonvibrant shadow");
    p.shadowVibrancyContribution=1;
    effect=Internal::makeNativeGlassEffect(p,UIGlassEffect::Style::regular,UIColor::clear,false);
    expect(effect.uniform("nativeBlurRadii")->at(3)==6,
        "native shadow samples use half radius only with active vibrancy");
}

void testMaterial() {
    auto result = SkRuntimeEffect::MakeForShader(SkString(UIGlassEffect(UIGlassEffect::Style::clear).backdropEffect().shaderSource()));
    expect(result.effect != nullptr, "complete liquid glass shader compiles with Skia");
    if (!result.effect) {
        std::cerr << result.errorText.c_str() << '\n';
        return;
    }
    runtime = result.effect;
    testNativeFaceHoldouts();
    testNativeHoldingComposition();
    testNativeTransparentMaterial();
    testNativeBlurRangePacking();
    testNativeTint();
    testNativeOpticalControls();
    testNativeProfiles();
    testGlassTintPreference();
    testNativeGlassTintControls();
    const auto clearBody = renderDefaultBody(UIGlassEffect::Style::clear, {132, 56});
    const auto smallBody = renderDefaultBody(UIGlassEffect::Style::regular, {132, 56});
    const auto largeBody = renderDefaultBody(UIGlassEffect::Style::regular, {320, 240});
    // The independent iOS27.1 preference fixture consistently measures32
    // for neutral clear glass over black. The old26 smoke value predated that
    // fixture and disagreed with all of its preferences/themes/sizes.
    expect(std::abs(luma(clearBody)-32)<=3
               && luma(smallBody) >= 130 && luma(smallBody) <= 137,
           "clear and small regular glass match the theme-fixed native black-backdrop samples");
    expect(luma(largeBody) >= 133 && luma(largeBody) <= 139,
           "large regular glass retains the native neutral body over black");
    const auto gray = SkColorSetRGB(128, 128, 128);
    expect(std::abs(luma(renderDefaultBody(UIGlassEffect::Style::regular, {132, 56}, gray)) - 199) <= 6,
           "small regular glass adapts to the native mid-gray sample rather than staying transparent");
    expect(std::abs(luma(renderDefaultBody(UIGlassEffect::Style::regular, {300, 160}, gray)) - 201) <= 6,
           "large regular glass preserves the native neutral tone over gray");
    const auto rest=render(true,false,0,{20,28},{20,28},UIColor::clear);
    const auto press=render(true,false,1,{20,28},{20,28},UIColor::clear);
    const auto far=render(true,false,1,{20,28},{110,28},UIColor::clear);
    expect(luma(press)>luma(rest)+20,"touch adds visible ambient light and a local sheen");
    expect(luma(press)>luma(far),"touch highlight follows the finger");
    expect(SkColorGetA(press)==255,"pressed material remains opaque over its sampled backdrop");
    for (const float scale:{1.f,2.f,3.f}) {
        expect(render(true,false,0,{20,28},{20,28},UIColor::clear,28,scale)==rest,
               "content scale changes antialiasing without changing the flat interior");
    }
    expect(SkColorGetA(render(false,false,1,{}, {66,28},UIColor::clear,0))==255,
           "zero-radius geometry remains finite and drawable");
    expect(SkColorGetA(render(false,false,1,{}, {66,28},UIColor::clear,28,1,SK_ColorTRANSPARENT))==255,
           "native interior material provides coverage over a transparent backdrop");
    expect(luma(render(true,true,1,{66,28},{66,28},UIColor::clear,28,1,SK_ColorBLACK))>10,
           "press feedback remains visible over black");
    UIGlassEffect identity;
    identity.setFrostRadius(0);identity.setRefraction(0);identity.setSpecularStrength(0);
    identity.setSaturation(1);identity.setContrast(1);identity.setTintOpacity(0);identity.setEdgeStrength(0);
    const auto resolved=GlassMaterialTestHarness::resolve(identity,{120,60},false);
    expect(resolved.backdropBlurRadius()==0,"public zero frost disables the main Gaussian");
    for (const auto& [name,radius]:resolved.backdropInputs())
        expect(radius==0,"public zero frost disables every auxiliary Gaussian");
    const auto refraction=resolved.uniform("nativeRefraction");
    expect(refraction->at(0)==0&&refraction->at(2)==0,"public zero refraction disables both optical displacement lanes");
    expect(resolved.uniform("nativeKeyHighlight")->at(2)==0
               &&resolved.uniform("nativeFillHighlight")->at(2)==0
               &&resolved.uniform("nativeFilterHighlight")->at(0)==0,
           "public zero specular strength disables key/fill and filter highlights");
    expect(renderResolvedBody(resolved,{120,60},SK_ColorRED)==SK_ColorRED,
           "explicit material identity preserves a solid-color source");
    for (int scale:{1,2,3}) {
        const auto sharp=renderFilterFixture(identity,scale);
        auto frosted=identity;frosted.setFrostRadius(7);
        const auto blurred=renderFilterFixture(frosted,scale);
        const auto centerY=(48+30)*scale;
        const auto sharpRed=sharp.getColor((48+50)*scale,centerY);
        const auto sharpBlue=sharp.getColor((48+54)*scale,centerY);
        const auto blurredRed=blurred.getColor((48+50)*scale,centerY);
        expect(SkColorGetR(sharpRed)>240&&SkColorGetB(sharpRed)<15
                   &&SkColorGetB(sharpBlue)>240&&SkColorGetR(sharpBlue)<15,
               "zero frost and zero refraction preserve independently drawn stripe geometry");
        expect(SkColorGetR(blurredRed)>50&&SkColorGetB(blurredRed)>50,
               "nonzero public frost mixes source stripes through the production filter graph");
        auto refracted=identity;refracted.setRefraction(.55f);
        const auto lens=renderFilterFixture(refracted,scale);
        long difference=0;
        for(int y=4;y<16;++y)for(int x=30;x<90;++x) {
            const auto a=sharp.getColor((48+x)*scale,(48+y)*scale);
            const auto b=lens.getColor((48+x)*scale,(48+y)*scale);
            difference+=std::abs(int(SkColorGetR(a))-int(SkColorGetR(b)));
        }
        expect(difference>100,"nonzero public refraction changes textured bevel samples independently of frost");
    }
}
}

int main() try {
    {
        auto description = UIGlassEffect(UIGlassEffect::Style::clear).backdropEffect();
        auto layer = new_shared<CABackdropEffectLayer>(description);
        const auto snapshot = std::static_pointer_cast<CABackdropEffectLayer>(layer->copy());
        const auto original = snapshot->effect().uniform("nativeFaceBlack");
        description.setUniform("nativeFaceBlack", .75f);
        layer->setEffect(description);
        expect(snapshot->effect().uniform("nativeFaceBlack") == original,
               "a presentation snapshot retains its original immutable material when the model changes");
        auto returned = layer->effect();
        returned.setUniform("nativeFaceBlack", .25f);
        expect(layer->effect().uniform("nativeFaceBlack")->front() == .75f,
               "editing a returned material description cannot mutate a shared render snapshot");
    }
    struct RasterBackend {
        sk_sp<SkSurface> surface(int w,int h){return SkSurfaces::Raster(SkImageInfo::MakeN32Premul(w,h));}
        std::vector<uint8_t> read(const sk_sp<SkSurface>& surface) {
            const auto info=SkImageInfo::Make(surface->width(),surface->height(),kRGBA_8888_SkColorType,kPremul_SkAlphaType);
            std::vector<uint8_t> bytes(info.computeMinByteSize());
            if(!surface->readPixels(info,bytes.data(),info.minRowBytes(),0,0))throw std::runtime_error("Press fixture readback failed");
            return bytes;
        }
    } backend;
    glassPressCoverageFixture(backend,[](const auto& effect,NXSize size,bool dark,int scale,float amount){
        return GlassMaterialTestHarness::resolve(effect,size,dark,amount,scale);
    });
    testMotion();
    testAppearancePolicy();
    testBackdropInputs();
    testMaterial();
    if (failures) return 1;
    std::cout << "Glass material and motion tests passed\n";
}
 catch(const std::exception& error) {
    std::cerr<<error.what()<<'\n';return 1;
}
