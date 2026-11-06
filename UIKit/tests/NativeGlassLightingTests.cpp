#include "NativeGlassLighting.hpp"
#include "NativeGlassProfile.hpp"
#include <BackdropEffect.h>
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkRuntimeEffect.h"
#include <cmath>
#include <iostream>
#include <string>

using namespace NXKit;
using namespace NXKit::Internal;
namespace {
int failures=0;
void expect(bool result,const char* message) {
    if (!result) { ++failures; std::cerr<<"FAIL: "<<message<<'\n'; }
}
sk_sp<SkRuntimeEffect> runtime;

NativeGlassProfile unlit() {
    auto p=nativeGlassProfile(UIGlassEffect::Style::regular,UIUserInterfaceStyle::dark,.5f);
    p.shadowOpacity=p.sdrShadowOpacity=p.shadowAmount=p.ringShadowOpacity=p.bleedOpacity=0;
    p.keyFillHighlightAmount=p.highlight.keyAmount=p.highlight.fillAmount=0;
    return p;
}
SkColor sample(const NativeGlassProfile& p, bool exterior, NXPoint position={},
               NXPoint normal={1,0},NXPoint worldNormal={1,0}, bool transparent=false,
               NXFloat displayHeadroom=1,bool interior=false,bool exactRing=false,
               NXFloat sourceGray=.5f,NXFloat highlightSDF=0,NXFloat bleedGray=.8f,int mode=-1) {
    BackdropEffect description("unused");
    bindNativeGlassLighting(p,description,displayHeadroom);
    SkRuntimeEffectBuilder builder(runtime);
    for (const auto& uniform:runtime->uniforms()) {
        const std::string name(uniform.name);
        auto value=description.uniform(name);
        if (value) builder.uniform(name.c_str()).set(value->data(),value->size());
    }
    builder.uniform("contentScale")=3.0f;
    builder.uniform("testExterior")=mode>=0?float(mode):(exterior?1.0f:(interior?2.0f:0.0f));
    builder.uniform("testPosition")=SkV2{position.x,position.y};
    builder.uniform("testNormal")=SkV2{normal.x,normal.y};
    builder.uniform("testWorldNormal")=SkV2{worldNormal.x,worldNormal.y};
    builder.uniform("testAlpha")=transparent?0.0f:1.0f;
    builder.uniform("testExactRing")=exactRing?1.0f:0.0f;
    builder.uniform("testGray")=sourceGray;
    builder.uniform("testBleedGray")=bleedGray;
    builder.uniform("testHighlightSDF")=highlightSDF;
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1,1));
    surface->getCanvas()->clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    paint.setShader(builder.makeShader());
    surface->getCanvas()->drawPaint(paint);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(1,1);
    surface->readPixels(bitmap,0,0);
    return bitmap.getColor(0,0);
}
int red(SkColor c) { return SkColorGetR(c); }
void testExterior() {
    auto p=unlit();
    expect(SkColorGetA(sample(p,true,{17,0}))==0,"zero exterior amounts produce transparent output");
    p.shadowOpacity=.8f;
    p.shadowColorMatrixFillColor={0,0,0,1};
    p.shadowOffset={8,0};
    p.shadowRadius=2;
    p.shadowBlurRadius=0;
    const auto shifted=sample(p,true,{18,0});
    p.shadowOffset={-8,0};
    const auto away=sample(p,true,{18,0});
    expect(SkColorGetA(shifted)>SkColorGetA(away)+80,"shadow follows actual shifted SDF geometry");
    p.shadowOffset={8,0};
    expect(SkColorGetA(sample(p,true,{18,0},{1,0},{1,0},true))==SkColorGetA(shifted),
           "native shadow coverage is independent of the backdrop texture alpha");
    expect(SkColorGetA(sample(p,true,{40,0}))==0,"native output maximum clips all exterior passes");
    p=unlit();p.ringShadowOpacity=.8f;p.ringShadowStrokeWidth=3;p.ringShadowBlurRadius=1;
    p.ringShadowOffset=0;
    const auto narrow=sample(p,false,{15,0},{1,0},{1,0},false,1,true);
    p.ringShadowBlurRadius=5;
    const auto broad=sample(p,false,{15,0},{1,0},{1,0},false,1,true);
    expect(red(narrow)<red(broad)&&red(narrow)<120,"ring width and blur independently shape the inner response");
    p=unlit();p.ringShadowOpacity=1;p.ringShadowStrokeWidth=1;
    p.ringShadowBlurRadius=.2f;p.ringShadowOffset=8;
    const auto tangentRing=sample(p,false,{13.5f,0},{1,0},{1,0},false,1,true);
    const auto exactRing=sample(p,false,{13.5f,0},{1,0},{1,0},false,1,true,true);
    expect(red(exactRing)+80<red(tangentRing),
           "offset ring evaluates the actual curved contour rather than a tangent expansion");
    p=unlit();p.bleedAmount=28;p.bleedHeight=28;p.bleedOpacity=.7f;
    p.bleedColorMatrixBlack=p.bleedColorMatrixWhite=0;
    const auto bleed=sample(p,false,{15,0},{1,0},{1,0},false,1,true);
    expect(red(bleed)<125,"bleed has its own inner color response independent of shadow opacity");
    expect(red(sample(p,false,{15,0},{1,0},{1,0},true,1,true))==red(bleed),
           "native bleed matrix blend retains its weight over a transparent source");
    p.bleedAmount=p.bleedHeight=0;
    expect(red(sample(p,false,{15,0},{1,0},{1,0},false,1,true))==red(bleed),
           "zero bleed displacement retains the independently filtered color blend");
    expect(SkColorGetA(sample(p,true,{18,0}))==0,"bleed does not create a broad exterior halo");
    p.bleedOpacity=1;p.bleedDarkenBlend=false;
    const auto invertedGain=sample(p,false,{15,0},{1,0},{1,0},false,1,true,false,.75f);
    p.bleedDarkenBlend=true;
    expect(red(sample(p,false,{15,0},{1,0},{1,0},false,1,true,false,.75f))+40<red(invertedGain),
           "native darken bleed selects the post-face luminance fourth-power response");
}
void testHoldingTone() {
    auto p=unlit();p.sdrHoldingToneEnabled=true;p.sdrHoldingToneWhite=.5f;
    p.sdrGradientDistance0=-1;p.sdrGradientDistance1=1;
    auto probe=[&](NXFloat x) {return red(sample(p,false,{x,0},{1,0},{1,0},false,
        1,false,false,.5f,0,.8f,4));};
    expect(std::abs(probe(15)-64)<=1,"holding tone operates on premultiplied material color");
    expect(std::abs(probe(16)-96)<=1,"holding tone interpolates linearly across native SDF stops");
    expect(std::abs(probe(17)-128)<=1,"holding tone leaves its far stop unchanged");
    p.sdrHoldingToneEnabled=false;
    expect(std::abs(probe(15)-128)<=1,"disabled holding tone preserves partial-alpha material");
}
void testExtendedRangeClamp() {
    auto p=unlit();p.clamp=1;p.clampPreserveHue=false;
    auto probe=[&] {return sample(p,false,{}, {1,0},{1,0},false,1,false,false,.5f,0,.8f,5);};
    auto c=probe();
    expect(std::abs(red(c)-64)<=1&&std::abs(int(SkColorGetG(c))-64)<=1,
           "internal material clamp retains negative components before composition");
    p.clamp=0;c=probe();
    expect(std::abs(red(c)-64)<=1&&std::abs(int(SkColorGetG(c))-128)<=1,
           "nonpositive native clamp limit disables clamping");
    p.clamp=1;p.clampPreserveHue=true;c=probe();
    expect(std::abs(red(c)-96)<=1&&std::abs(int(SkColorGetB(c))-64)<=1,
           "hue preserving clamp scales negative and positive components together");
}
void testNativeFilterFlags() {
    auto p=unlit();
    auto flags=[&] {
        BackdropEffect description("unused");bindNativeGlassLighting(p,description);
        return *description.uniform("nativeFilterOptions");
    };
    p.keyFillHighlightColorBias=-.1f;p.keyFillHighlightEffectOffset=-.5f;
    p.keyFillHighlightHeight=.5f;
    expect(flags()==std::vector<NXFloat>({1,1,1}),
           "native filter enables bias correction and extension for captured defaults");
    p.keyFillHighlightHeight=.6f;
    expect(flags()==std::vector<NXFloat>({1,1,0}),
           "native stroke mode requires the height and outside offset to cancel");
    p.keyFillHighlightColorBias=0;p.keyFillHighlightEffectOffset=0;
    expect(flags()==std::vector<NXFloat>({0,0,0}),
           "zero bias and nonnegative offset disable their native filter flags");
    p.keyFillHighlightColorBias=.1f;p.keyFillHighlightEffectOffset=.5f;
    expect(flags()==std::vector<NXFloat>({1,0,0}),
           "native bias correction tests nonzero bias independently of outside extension");
}
void testNativeUniformBleed() {
    // Entire framebuffer uses .5gray. No neighboring color bands contribute.
    // Baseline/no-bleed centers: light44/80/120=195/197/200 vs195/195/195;
    // dark=134/127/117 vs134/128/118. Every row repeats the same native result.
    for(bool dark:{false,true})for(int index=0;index<3;index++) {
        const NXFloat sizes[]={44,80,120};
        const int unlitDark[]={134,128,118},litDark[]={134,127,117};
        const int litLight[]={195,197,200};
        auto p=nativeGlassProfile(UIGlassEffect::Style::regular,
            dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light,.5f,
            {sizes[index],sizes[index]},false);
        p.ringShadowOpacity=0;
        const int backdrop=dark?unlitDark[index]:195;
        const int expected=dark?litDark[index]:litLight[index];
        const auto actual=sample(p,false,{}, {0,-1},{0,-1},false,1,true,false,
                                 backdrop/255.f,0,.5f);
        expect(std::abs(red(actual)-expected)<=2,
               "native uniform bleed persists across the center of larger materials");
    }
}
void testNativeSpatialHighlight() {
    // Independent UIKit framebuffer pairs: normal / SDF-highlight opacity0.
    // Samples are >=.5pt inside, beyond geometric antialiasing.44pt dark was
    // initially calibrated empirically; AIR equations now replace that fit. These test
    // only SDF lighting over its measured backdrop, not whole-image parity.
    struct Control {bool dark;NXFloat size,sdf;int backdrop,expected;};
    const Control controls[]={
        {true,44,-5.1666667f,31,32},{true,44,-2.1666667f,32,38},{true,44,-.5f,32,73},
        {true,80,-5.1666667f,126,127},{true,80,-2.1666667f,127,132},{true,80,-.5f,127,158},
        {true,120,-5.1666667f,120,121},{true,120,-2.1666667f,121,126},{true,120,-.5f,121,153},
        {false,44,-5.1666667f,131,132},{false,44,-2.1666667f,132,137},{false,44,-.5f,132,162},
        {false,80,-5.1666667f,194,195},{false,80,-2.1666667f,195,199},{false,80,-.5f,196,220},
        {false,120,-5.1666667f,248,248},{false,120,-2.1666667f,249,250},{false,120,-.5f,249,255}
    };
    for(const auto& c:controls) {
        auto p=nativeGlassProfile(UIGlassEffect::Style::regular,
            c.dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light,.5f,{c.size,c.size},false);
        p.keyFillHighlightAmount=p.shadowAmount=0;
        const auto actual=sample(p,false,{}, {0,-1},{0,-1},false,1,false,false,
                                 c.backdrop/255.f,c.sdf);
        expect(std::abs(red(actual)-c.expected)<=4,
               "native spatial SDF reflection matches independent byte controls");
    }
}
void testHighlight() {
    auto p=unlit();
    expect(std::abs(red(sample(p,false))-128)<=1,"disabled highlight and internal shadow retain source color");
    p.highlight.keyAmount=.8f;p.highlight.keyAngle=0;p.highlight.keyHeight=3;
    // A controlled constant-white vibrant transform isolates geometry from
    // the captured, backdrop-dependent native color response.
    p.highlight.vibrant.matrix.values={0,0,0,0,1, 0,0,0,0,1,
                                      0,0,0,0,1, 0,0,0,1,0};
    const auto lit=sample(p,false,{}, {0,-1});
    const auto dark=sample(p,false,{}, {0,1});
    expect(red(lit)>red(dark)+40,"key highlight responds to normal direction");
    p.highlight.global=true;
    expect(red(sample(p,false,{}, {0,1},{0,-1}))>red(dark)+40,
           "global light uses the supplied world normal");
    p=unlit();p.highlight.keyAmount=.8f;p.highlight.keyAngle=0;
    p.highlight.keyHeight=3;p.highlight.diffuseAmountScale=0;
    const auto vibrant=sample(p,false,{}, {0,-1});
    const auto& matrix=p.highlight.vibrant.matrix.values;
    const float mapped=.5f*(matrix[0]+matrix[1]+matrix[2])+matrix[3]+matrix[4];
    const int expected=std::lround(255*(.5f+.8f*(mapped-.5f)));
    expect(std::abs(red(vibrant)-expected)<=1,
           "native highlight transforms the backdrop and adds its matrix difference");
    p.highlight.keyAmount=2;
    expect(std::abs(red(sample(p,false,{}, {0,-1}))-std::lround(mapped*255))<=1,
           "bias amounts beyond one clamp to the valid Schlick endpoint");
    p.highlight.keyAmount=.5f;p.highlight.keyHeight=1;p.highlight.diffuseAmountScale=.15f;
    const auto combined=sample(p,false,{}, {0,-1},{0,-1},false,1,false,false,.5f,-.5f);
    expect(red(combined)>std::lround(mapped*255)+2,
           "primary and diffuse native bias responses can sum beyond one");
    p=unlit();p.shadowAmount=50;p.shadowHeight=32;
    p.shadowColorMatrixFillColor={0,0,0,.5f};
    expect(std::abs(red(sample(p,false,{}, {0,1}))-128)<=1,
           "native shadow amount displaces the exterior sample and does not darken the interior");
    p=unlit();p.keyFillHighlightAmount=.8f;p.keyFillHighlightHeight=2;
    p.keyFillHighlightEffectOffset=-2;p.keyFillHighlightAngle=0;
    p.keyFillHighlightSpread=.1f;p.keyFillHighlightSpreadSDR=3.0f;
    const auto sdr=sample(p,false,{17,0}, {1,0},{1,0},false,1,false,false,.5f,0,.8f,3);
    const auto hdr=sample(p,false,{17,0}, {1,0},{1,0},false,4,false,false,.5f,0,.8f,3);
    expect(red(sdr)<red(hdr)-5,"actual display headroom selects the outside filter's SDR spread");
}
}
int main() {
    const std::string source=std::string("uniform float contentScale;\n")+
        std::string(nativeGlassLightingUniforms())+std::string(nativeGlassLightingShaderSource())+R"sksl(
uniform float testExterior;
uniform float2 testPosition;
uniform float2 testNormal;
uniform float2 testWorldNormal;
uniform float testAlpha;
uniform float testExactRing;
uniform float testGray;
uniform float testBleedGray;
uniform float testHighlightSDF;
half4 main(float2 xy) {
    if(testExterior>4.5) {
        float3 c=nativeLightingClampColor(float3(-.5,2,.5),nativeLightingClamp.x,nativeLightingClamp.y);
        return half4(.5+c.r*.5,c.g*.25,c.b,1);
    }
    if(testExterior>3.5) {
        return nativeGlassHoldingTone(length(testPosition)-16.0,half4(.2,.2,.2,.4));
    }
    if(testExterior>2.5) {
        half4 source=half4(half3(testGray*testAlpha),half(testAlpha));
        half4 field=nativeGlassBackgroundFinalize(length(testPosition)-16.0,half4(0),source,
            half3(testGray),0,testNormal,3);
        return nativeLightingOver(field,source);
    }
    if(testExterior>1.5) {
        half4 bleed=half4(half3(testBleedGray*testAlpha),half(testAlpha));
        float sdf=length(testPosition)-16.0;
        float ring=length(testPosition-float2(0.0,nativeRingShadowOffset))-16.0;
        half4 original=half4(half3(.5*testAlpha),half(testAlpha));
        return half4(nativeGlassInterior(sdf,testNormal,half3(testGray),bleed,float2(32),
            testExactRing>.5?ring:sdf-testNormal.y*nativeRingShadowOffset,original),1.0);
    }
    if (testExterior>0.5) {
        float sdf=length(testPosition)-16.0;
        float shifted=length(testPosition-nativeShadowOffset)-16.0;
        float ring=length(testPosition-float2(0.0,nativeRingShadowOffset))-16.0;
        half4 source=half4(half3(0.5*testAlpha),half(testAlpha));
        half4 bleed=half4(half3(0.8*testAlpha),half(testAlpha));
        return nativeGlassExterior(sdf,shifted,ring,testNormal,source,source,bleed);
    }
    return half4(nativeGlassHighlight(testHighlightSDF,testNormal,testWorldNormal,half3(testGray)),1.0);
})sksl";
    auto compiled=SkRuntimeEffect::MakeForShader(SkString(source));
    if (!compiled.effect) { std::cerr<<compiled.errorText.c_str()<<'\n';return 1; }
    runtime=std::move(compiled.effect);
    testExterior();testHighlight();testNativeSpatialHighlight();testNativeUniformBleed();testHoldingTone();testExtendedRangeClamp();testNativeFilterFlags();
    if (failures) return 1;
    std::cout<<"Native glass lighting tests passed\n";
}
