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
        return UIGlassEffect(style)._backdropEffect(size, UITraitCollection::current());
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
    expect(std::abs(pill.m11 * 132 - 149) < 0.01,
           "press grows the longest side by 17 points");
    expect(std::abs(held.transform({56, 56}).m11 * 56 - 73) < 0.01,
           "circle press uses the same point growth as the pill");
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
        "tint", "edge", "specStrength", "specFallbackStrength", "specBrightFallbackStrength",
        "specLightDirection", "specDirectionalPower", "specWidthPx", "reflectionSamplePx",
        "reflectionSpreadPx", "brightBackdropShade", "referenceAppearance", "bodyMode",
        "backdropLuma", "edgeAbsorption", "adaptiveTint"
    }) {
        const auto components = glass.uniform(name);
        if (components) builder.uniform(name).set(components->data(), components->size());
    }
    builder.uniform("resolution") = SkV2{size.width, size.height};
    builder.uniform("lensSize") = SkV2{size.width, size.height};
    builder.uniform("lensCenter") = SkV2{size.width * 0.5f, size.height * 0.5f};
    builder.uniform("cornerRadius") = radius;
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
    auto glass = BackdropEffect::glass();
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
    auto glass = BackdropEffect::glass();
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

SkColor renderDefaultBody(UIGlassEffect::Style style, NXSize size, SkColor background = SK_ColorBLACK) {
    const auto effect = GlassMaterialTestHarness::resolve(style, size);
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
    return pixels.getColor(size.width / 2, size.height / 2);
}

void testMaterial() {
    auto result = SkRuntimeEffect::MakeForShader(SkString(BackdropEffect::glass().shaderSource()));
    expect(result.effect != nullptr, "complete liquid glass shader compiles with Skia");
    if (!result.effect) {
        std::cerr << result.errorText.c_str() << '\n';
        return;
    }
    runtime = result.effect;
    const auto clearBody = renderDefaultBody(UIGlassEffect::Style::clear, {132, 56});
    const auto smallBody = renderDefaultBody(UIGlassEffect::Style::regular, {132, 56});
    const auto largeBody = renderDefaultBody(UIGlassEffect::Style::regular, {320, 240});
    expect(luma(clearBody) >= 17 && luma(clearBody) <= 22
               && luma(smallBody) >= 17 && luma(smallBody) <= 22,
           "clear and small regular glass match the native black-backdrop body samples");
    expect(luma(largeBody) >= 174 && luma(largeBody) <= 184,
           "large regular glass matches the native 179/255 neutral body over black");
    const auto gray = SkColorSetRGB(128, 128, 128);
    expect(std::abs(luma(renderDefaultBody(UIGlassEffect::Style::regular, {132, 56}, gray)) - 205) <= 6,
           "small regular glass adapts to the native mid-gray sample rather than staying transparent");
    expect(std::abs(luma(renderDefaultBody(UIGlassEffect::Style::regular, {300, 160}, gray)) - 219) <= 6,
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
