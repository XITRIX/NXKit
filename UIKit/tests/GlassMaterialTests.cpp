#include "LayerCornerGeometry.hpp"
#include <BackdropEffect.h>
#include <UIGlassEffect.h>
#include <UITraitCollection.h>
#include "GlassInteraction.hpp"
#include "GlassAppearance.hpp"

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
    static BackdropEffect resolve(const UIGlassEffect& effect, NXSize size, bool dark) {
        auto traits=std::make_shared<UITraitCollection>();
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
    for (const auto* name : {
        "refraction", "curve", "dispersion", "saturation", "contrast", "luminosity",
        "tint", "tintMode", "edge", "specStrength", "specFallbackStrength", "specBrightFallbackStrength",
        "specLightDirection", "specDirectionalPower", "specWidthPx", "reflectionSamplePx",
        "reflectionSpreadPx", "brightBackdropShade", "referenceAppearance", "bodyMode",
        "backdropLuma", "edgeAbsorption", "adaptiveTint", "bodyCompression", "neutralDetail", "materialFrost"
    }) {
        const auto components = glass.uniform(name);
        if (components) builder.uniform(name).set(components->data(), components->size());
    }
    builder.uniform("resolution") = SkV2{size.width, size.height};
    builder.uniform("lensSize") = SkV2{size.width, size.height};
    builder.uniform("lensCenter") = SkV2{size.width * 0.5f, size.height * 0.5f};
    builder.uniform("cornerRadius") = radius;
    Internal::bindLayerCornerGeometry(builder,*runtime,size,radius);
    builder.uniform("contentScale") = scale;
    builder.uniform("filterScale") = 1.0f;
    builder.uniform("maximumSampleRadius") = glass.maximumSampleRadius();
}

SkBitmap renderFilterFixture(int scale, bool lineOnly, bool regular = false) {
    constexpr int padding = 32, width = 120, height = 60;
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(
        (width + padding * 2) * scale, (height + padding * 2) * scale
    ));
    auto* canvas = surface->getCanvas();
    canvas->scale(scale, scale);
    canvas->clear(lineOnly ? SK_ColorBLACK : SK_ColorBLUE);
    if (!lineOnly) {
        SkPaint red;
        red.setColor(SK_ColorRED);
        // This red band lies inside the lens, far from the inspected top edge.
        // The native-calibrated fold reaches this band near the top perimeter.
        canvas->drawRect(SkRect::MakeXYWH(0, padding + 34, width + padding * 2, 6), red);
    }
    auto glass = UIGlassEffect(UIGlassEffect::Style::clear).backdropEffect();
    glass.setUniform("bodyMode", regular ? 0.0f : 1.0f);
    glass.setUniform("tint", UIColor::clear);
    glass.setUniform("saturation", 1.0f);
    glass.setUniform("contrast", 1.0f);
    glass.setUniform("luminosity", 0.0f);
    glass.setUniform("specStrength", 0.0f);
    glass.setUniform("edgeAbsorption", 0.0f);
    glass.setUniform("brightBackdropShade", 0.0f);
    if (lineOnly) {
        glass.setUniform("curve", 0.0f);
        glass.setUniform("dispersion", 0.0f);
    }
    SkRuntimeEffectBuilder builder(runtime);
    bindMaterial(builder, glass, {width, height}, height * 0.5f, scale);
    builder.uniform("filterScale") = static_cast<float>(scale);
    builder.uniform("interactionIntensity") = 0.0f;
    builder.uniform("interactionLocation") = SkV2{0, 0};
    std::string_view names[] = {"content", "sharpContent"};
    const sk_sp<SkImageFilter> inputs[] = {
        SkImageFilters::Blur((regular ? 24 : 2) * scale, (regular ? 24 : 2) * scale, SkTileMode::kClamp, nullptr), nullptr
    };
    auto filter = SkImageFilters::RuntimeShader(builder, glass.maximumSampleRadius() * scale, names, inputs, 2);
    expect(filter != nullptr, "sharp lens and frosted body bind as independent filter inputs");
    canvas->save();
    canvas->translate(padding, padding);
    canvas->scale(1.0f / scale, 1.0f / scale);
    const auto rect = SkRect::MakeWH(width * scale, height * scale);
    canvas->clipRRect(SkRRect::MakeRectXY(rect, height * scale * 0.5f, height * scale * 0.5f), true);
    SkPaint paint;
    paint.setImageFilter(filter);
    canvas->saveLayer(SkCanvas::SaveLayerRec(&rect, &paint, SkCanvas::kInitWithPrevious_SaveLayerFlag));
    canvas->restore();
    canvas->restore();
    SkBitmap pixels;
    pixels.allocN32Pixels(surface->width(), surface->height());
    expect(surface->readPixels(pixels, 0, 0), "the actual image-filter graph produces readable pixels");
    return pixels;
}

SkColor render(bool clear, bool dark, NXFloat glow, NXPoint touch, NXPoint sample,
               const UIColor& tint, NXFloat radius = 28, NXFloat scale = 1,
               SkColor background = SkColorSetRGB(70, 100, 125)) {
    auto glass = UIGlassEffect(UIGlassEffect::Style::clear).backdropEffect();
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(132, 56));
    SkRuntimeEffectBuilder builder(runtime);
    // Start from the same clear material used by UIGlassEffect; override only
    // the settings needed to isolate each optical assertion.
    bindMaterial(builder, glass, {132, 56}, radius, scale);
    builder.uniform("referenceAppearance") = 1.0f;
    builder.uniform("bodyMode") = clear ? 1.0f : 0.0f;
    builder.uniform("backdropLuma") = dark ? 0.15f : 0.85f;
    builder.uniform("specDirectionalPower") = 16.0f;
    builder.uniform("specWidthPx") = 0.5f;
    builder.uniform("saturation") = 1.0f;
    builder.uniform("contrast") = 1.0f;
    builder.uniform("tint") = SkV4{tint.r() / 255.0f, tint.g() / 255.0f, tint.b() / 255.0f, tint.a() / 255.0f};
    builder.uniform("interactionIntensity") = glow;
    builder.uniform("interactionLocation") = SkV2{touch.x, touch.y};
    builder.child("content") = SkShaders::Color(background);
    builder.child("sharpContent") = SkShaders::Color(background);
    SkPaint paint;
    paint.setShader(builder.makeShader());
    surface->getCanvas()->drawPaint(paint);
    SkBitmap pixels;
    pixels.allocN32Pixels(132, 56);
    expect(surface->readPixels(pixels, 0, 0), "shader output can be read on the raster backend");
    return pixels.getColor(sample.x, sample.y);
}

int luma(SkColor color) {
    return SkColorGetR(color) * 0.2126 + SkColorGetG(color) * 0.7152 + SkColorGetB(color) * 0.0722;
}

SkColor renderResolvedBody(const BackdropEffect& effect, NXSize size, SkColor background = SK_ColorBLACK, NXPoint sample = {-1, -1}) {
    SkRuntimeEffectBuilder builder(runtime);
    bindMaterial(builder, effect, size, std::min(size.width, size.height) * 0.5f, 1);
    builder.uniform("interactionIntensity") = 0.0f;
    builder.uniform("interactionLocation") = SkV2{0, 0};
    builder.child("content") = SkShaders::Color(background);
    builder.child("sharpContent") = SkShaders::Color(background);
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
        expect(override.uniform("tintMode")->front()==0 && override.uniform("tint")->back()==0,"explicit opacity overrides retain the low-level tint path");
    }
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
    auto resolved = GlassMaterialTestHarness::resolve(glass,{52,52},false);
    const auto background = SkColorSetRGB(128,128,128);
    const auto top = renderResolvedBody(resolved,{52,52},background,{26,0});
    const auto side = renderResolvedBody(resolved,{52,52},background,{0,26});
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

void testMaterial() {
    auto result = SkRuntimeEffect::MakeForShader(SkString(UIGlassEffect(UIGlassEffect::Style::clear).backdropEffect().shaderSource()));
    expect(result.effect != nullptr, "complete liquid glass shader compiles with Skia");
    if (!result.effect) {
        std::cerr << result.errorText.c_str() << '\n';
        return;
    }
    runtime = result.effect;
    testNativeTint();
    testNativeOpticalControls();
    const auto clearBody = renderDefaultBody(UIGlassEffect::Style::clear, {132, 56});
    const auto smallBody = renderDefaultBody(UIGlassEffect::Style::regular, {132, 56});
    const auto largeBody = renderDefaultBody(UIGlassEffect::Style::regular, {320, 240});
    expect(luma(clearBody) >= 23 && luma(clearBody) <= 28
               && luma(smallBody) >= 130 && luma(smallBody) <= 137,
           "clear and small regular glass match the theme-fixed native black-backdrop samples");
    expect(luma(largeBody) >= 133 && luma(largeBody) <= 139,
           "large regular glass retains the native neutral body over black");
    const auto gray = SkColorSetRGB(128, 128, 128);
    expect(std::abs(luma(renderDefaultBody(UIGlassEffect::Style::regular, {132, 56}, gray)) - 199) <= 6,
           "small regular glass adapts to the native mid-gray sample rather than staying transparent");
    expect(std::abs(luma(renderDefaultBody(UIGlassEffect::Style::regular, {300, 160}, gray)) - 201) <= 6,
           "large regular glass preserves the native neutral tone over gray");
    const auto rest = render(true, false, 0, {20, 28}, {20, 28}, UIColor::clear);
    const auto press = render(true, false, 1, {20, 28}, {20, 28}, UIColor::clear);
    const auto far = render(true, false, 1, {20, 28}, {110, 28}, UIColor::clear);
    expect(luma(press) > luma(rest) + 20, "touch adds visible ambient light and a local sheen");
    expect(luma(press) > luma(far), "touch highlight follows the finger");
    expect(SkColorGetA(press) == 255, "pressed material remains opaque over its sampled backdrop");
    const auto untinted = render(false, false, 0, {}, {66, 28}, UIColor::clear);
    const auto tinted = render(false, false, 0, {}, {66, 28}, UIColor(30, 60, 180, 100));
    expect(std::abs(luma(tinted) - luma(untinted)) < 6,
           "regular chromatic tint preserves transmitted luminance");
    expect(SkColorGetB(tinted) > SkColorGetB(untinted), "regular tint changes hue");
    const auto clearTint = render(true, false, 0, {}, {66, 28}, UIColor(10, 20, 40, 100));
    expect(luma(clearTint) < luma(rest), "clear tint uses direct alpha compositing");
    const auto lightRim = render(true, false, 0, {}, {66, 0}, UIColor::clear);
    const auto darkRim = render(true, true, 0, {}, {66, 0}, UIColor::clear);
    expect(luma(darkRim) > luma(rest) && luma(lightRim) > luma(rest),
           "the native-style reflection lights the rim in both appearances");
    for (const float scale : {1.0f, 2.0f, 3.0f}) {
        expect(render(true, false, 0, {}, {66, 28}, UIColor::clear, 28, scale) == rest,
               "content scale changes antialiasing without changing the flat interior");
    }
    const auto corner = render(true, false, 1, {}, {0, 0}, UIColor::clear);
    expect(corner == SkColorSetRGB(70, 100, 125), "rounded-corner pixels preserve the backdrop");
    expect(SkColorGetA(render(false, false, 1, {}, {66, 28}, UIColor::clear, 0)) == 255,
           "zero-radius geometry remains finite and drawable");
    expect(luma(render(true, false, 0, {}, {66, 28}, UIColor::clear, 28, 1, SK_ColorWHITE)) > 245,
           "clear glass keeps a white backdrop clear");
    expect(luma(render(true, true, 1, {66, 28}, {66, 28}, UIColor::clear, 28, 1, SK_ColorBLACK)) > 10,
           "press feedback remains visible over black");

    for (int scale : {1, 2, 3}) {
        const auto lens = renderFilterFixture(scale, false);
        const auto folded = lens.getColor((32 + 60) * scale, 32 * scale + scale);
        const auto interior = lens.getColor((32 + 60) * scale, (32 + 30) * scale);
        expect(SkColorGetR(folded) > SkColorGetB(folded) + 60,
               "clear lens folds the interior stripe with native-calibrated gentle frost");
        expect(SkColorGetB(interior) > SkColorGetR(interior) + 60,
               "flat lens interior remains undisplaced");

        const auto regular = renderFilterFixture(scale, false, true);
        const auto frostedEdge = regular.getColor((32 + 60) * scale, 32 * scale + scale);
        expect(SkColorGetR(frostedEdge) < SkColorGetR(folded) - 100
                   && SkColorGetB(frostedEdge) > SkColorGetR(frostedEdge) + 60,
               "regular glass keeps frost through the bevel instead of exposing a clear edge band");

        const auto hairline = renderFilterFixture(scale, true);
        double coverage = 0;
        for (int x = 0; x < 4 * scale; ++x) {
            const auto color = hairline.getColor(32 * scale + x, (32 + 30) * scale);
            coverage += SkColorGetR(color) / (255.0 * 0.52);
        }
        const double width = coverage / scale;
        if (width <= 0.70 || width >= 0.90) {
            std::cerr << "Rim width at " << scale << "x: " << width << " points; samples:";
            for (int x = 0; x < 4 * scale; ++x) {
                std::cerr << ' ' << SkColorGetR(hairline.getColor(32 * scale + x, (32 + 30) * scale));
            }
            std::cerr << '\n';
        }
        expect(width > 0.70 && width < 0.90,
               "rim coverage stays sub-point at 1x, 2x and 3x rather than widening with AA");
    }
}
}

int main() {
    testMotion();
    testAppearancePolicy();
    testMaterial();
    if (failures) return 1;
    std::cout << "Glass material and motion tests passed\n";
}
