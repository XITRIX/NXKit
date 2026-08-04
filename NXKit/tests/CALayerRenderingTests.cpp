#include <BackdropEffect.h>
#include <BackdropEffectView.h>
#include <CALayer.h>
#include <UIGlassEffect.h>

#include "CABackdropEffectLayer.hpp"

#include "include/core/SkBitmap.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/utils/SkNoDrawCanvas.h"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace NXKit;

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (condition) return;
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}

struct SaveLayerCall {
    std::optional<SkRect> bounds;
    SkCanvas::SaveLayerFlags flags = 0;
    sk_sp<SkImageFilter> imageFilter;
    NXFloat alpha = 1;
};

class SaveLayerTrackingCanvas final: public SkNoDrawCanvas {
public:
    SaveLayerTrackingCanvas(int width, int height): SkNoDrawCanvas(width, height) {}

    std::vector<SaveLayerCall> calls;

protected:
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& record) override {
        calls.push_back({
            record.fBounds ? std::optional(*record.fBounds) : std::nullopt,
            record.fSaveLayerFlags,
            record.fPaint ? record.fPaint->refImageFilter() : nullptr,
            record.fPaint ? record.fPaint->getAlphaf() : 1.0f,
        });
        return SkNoDrawCanvas::getSaveLayerStrategy(record);
    }
};

void testMaskLayersUseLocalBounds() {
    auto layer = new_shared<CALayer>();
    layer->setBounds(NXRect(7, 9, 80, 40));
    layer->setPosition({120, 90});

    auto mask = new_shared<CALayer>();
    mask->setBounds(NXRect(0, 0, 80, 40));
    mask->setPosition({40, 20});
    layer->setMask(mask);

    SaveLayerTrackingCanvas canvas(640, 480);
    layer->skiaRender(&canvas);

    expect(canvas.calls.size() == 2,
           "a masked layer uses one content layer and one destination-in layer");
    const auto expectedBounds = SkRect::MakeWH(80, 40);
    for (const auto& call : canvas.calls) {
        expect(call.bounds.has_value(), "masked offscreen layers have explicit bounds");
        expect(call.bounds && *call.bounds == expectedBounds,
               "masked offscreen layers are bounded in layer-local coordinates");
    }
    if (!canvas.calls.empty()) {
        expect((canvas.calls.front().flags
                    & SkCanvas::kInitWithPrevious_SaveLayerFlag) != 0,
               "masked content retains the existing backdrop");
    }
}

void testOpacityLayersRetainBackdropForEffects() {
    auto layer = new_shared<CABackdropEffectLayer>(BackdropEffect::glass());
    layer->setBounds(NXRect(0, 0, 240, 120));
    layer->setOpacity(0.5f);

    SaveLayerTrackingCanvas canvas(640, 480);
    layer->skiaRender(&canvas);

    expect(
        canvas.calls.size() == 2,
        "partially opaque backdrop content uses opacity and effect layers"
    );
    if (canvas.calls.size() != 2) return;

    const auto& opacityLayer = canvas.calls.front();
    expect(
        opacityLayer.bounds == std::optional(SkRect::MakeWH(240, 120)),
        "the opacity group is bounded in layer-local coordinates"
    );
    expect(
        (opacityLayer.flags & SkCanvas::kInitWithPrevious_SaveLayerFlag) != 0,
        "an opacity group preserves pixels for its nested backdrop effect"
    );
    expect(
        std::abs(opacityLayer.alpha - 0.5f) < 0.001f,
        "the backdrop-preserving opacity group retains the requested alpha"
    );
    expect(
        canvas.calls.back().imageFilter != nullptr,
        "the nested backdrop effect still installs its runtime filter"
    );

    constexpr const char* invertBackdropShader = R"SKSL(
uniform shader content;

half4 main(float2 xy) {
    half4 source = content.eval(xy);
    return half4(half3(1.0) - source.rgb, source.a);
}
)SKSL";
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(20, 20));
    expect(surface != nullptr, "the opacity regression creates a raster surface");
    if (!surface) return;
    surface->getCanvas()->clear(SK_ColorRED);

    auto rasterLayer = new_shared<CABackdropEffectLayer>(BackdropEffect(
        invertBackdropShader,
        "content",
        0,
        0
    ));
    rasterLayer->setBounds(NXRect(0, 0, 20, 20));
    rasterLayer->setPosition(NXPoint(10, 10));
    rasterLayer->setOpacity(0.5f);
    rasterLayer->skiaRender(surface->getCanvas());

    SkBitmap pixels;
    pixels.allocN32Pixels(20, 20);
    expect(
        surface->readPixels(pixels, 0, 0),
        "the opacity regression reads its filtered result"
    );
    const auto center = pixels.getColor(10, 10);
    expect(
        SkColorGetG(center) > 80 && SkColorGetB(center) > 80,
        "a partial-opacity backdrop effect visibly filters previous pixels"
    );
}

void testBackdropFilterCacheTracksGeometry() {
    auto layer = new_shared<CABackdropEffectLayer>(BackdropEffect::glass());
    layer->setBounds(NXRect(0, 0, 340, 140));
    layer->setCornerRadius(36);

    SaveLayerTrackingCanvas canvas(640, 480);
    layer->draw(&canvas);
    expect(canvas.calls.size() == 1 && canvas.calls.back().imageFilter,
           "backdrop drawing creates a runtime image filter");
    if (canvas.calls.empty() || !canvas.calls.back().imageFilter) return;
    const auto initialFilter = canvas.calls.back().imageFilter;

    layer->draw(&canvas);
    expect(canvas.calls.back().imageFilter.get() == initialFilter.get(),
           "unchanged backdrop geometry reuses its image filter");

    layer->setPosition({100, 50});
    layer->draw(&canvas);
    expect(canvas.calls.back().imageFilter.get() == initialFilter.get(),
           "position-only changes reuse the backdrop image filter");

    layer->setBounds(NXRect(0, 0, 300, 140));
    layer->draw(&canvas);
    const auto resizedFilter = canvas.calls.back().imageFilter;
    expect(resizedFilter.get() != initialFilter.get(),
           "backdrop size changes rebuild the image filter");

    layer->setCornerRadius(24);
    layer->draw(&canvas);
    const auto roundedFilter = canvas.calls.back().imageFilter;
    expect(roundedFilter.get() != resizedFilter.get(),
           "backdrop corner-radius changes rebuild the image filter");

    layer->setContentsScale(2);
    layer->draw(&canvas);
    const auto scaledFilter = canvas.calls.back().imageFilter;
    expect(scaledFilter.get() != roundedFilter.get(),
           "backdrop content-scale changes rebuild the image filter");

    auto changedEffect = BackdropEffect::glass();
    changedEffect.setUniform("saturation", 0.9f);
    layer->setEffect(changedEffect);
    layer->draw(&canvas);
    expect(canvas.calls.back().imageFilter.get() != scaledFilter.get(),
           "backdrop effect changes rebuild the image filter");
}

struct GlassSamples {
    SkColor leftRim = SK_ColorTRANSPARENT;
    SkColor topRim = SK_ColorTRANSPARENT;
    SkColor bottomRim = SK_ColorTRANSPARENT;
    SkColor center = SK_ColorTRANSPARENT;
};

int brightness(SkColor color) {
    return SkColorGetR(color) + SkColorGetG(color) + SkColorGetB(color);
}

NXFloat scalarUniform(const BackdropEffect& effect, const std::string& name) {
    const auto values = effect.uniform(name);
    if (!values || values->size() != 1) {
        expect(false, "glass uniform '" + name + "' is a scalar");
        return 0;
    }
    return values->front();
}

std::vector<NXFloat> vectorUniform(
    const BackdropEffect& effect,
    const std::string& name,
    size_t expectedSize
) {
    const auto values = effect.uniform(name);
    if (!values || values->size() != expectedSize) {
        expect(false, "glass uniform '" + name + "' has the expected shape");
        return std::vector<NXFloat>(expectedSize);
    }
    return *values;
}

void testGlassStylesResolveThroughTheirView() {
    auto clearView = new_shared<BackdropEffectView>(
        UIGlassEffect(UIGlassEffect::Style::clear)
    );
    clearView->setFrame(NXRect(0, 0, 64, 64));
    const auto smallClear = clearView->effect();
    clearView->setFrame(NXRect(0, 0, 320, 320));
    const auto largeClear = clearView->effect();

    expect(
        std::abs(
            smallClear.backdropBlurRadius() - largeClear.backdropBlurRadius()
        ) < 0.001f,
        "clear glass keeps a size-invariant frost radius"
    );
    expect(
        vectorUniform(smallClear, "tint", 4)
            == vectorUniform(largeClear, "tint", 4),
        "clear glass keeps a size-invariant translucent tint"
    );

    auto regularView = new_shared<BackdropEffectView>(
        UIGlassEffect(UIGlassEffect::Style::regular)
    );
    regularView->setFrame(NXRect(0, 0, 64, 64));
    const auto smallRegular = regularView->effect();
    const std::shared_ptr<UIView> regularAsView = regularView;
    regularAsView->setFrame(NXRect(0, 0, 320, 320));
    const auto largeRegular = regularView->effect();
    const auto smallRegularTint = vectorUniform(smallRegular, "tint", 4);
    const auto largeRegularTint = vectorUniform(largeRegular, "tint", 4);

    expect(
        largeRegular.backdropBlurRadius()
            > smallRegular.backdropBlurRadius() + 15.0f,
        "regular glass increases backdrop frost with view size"
    );
    expect(
        largeRegularTint[3] > smallRegularTint[3] + 0.20f,
        "regular glass increases its system-background tint with view size"
    );
    expect(
        largeRegularTint[3] < 0.30f,
        "large regular glass stays translucent in light appearance"
    );
    expect(
        scalarUniform(largeRegular, "luminosity") > 0.10f,
        "large regular glass lifts luminosity in light appearance"
    );
    expect(
        largeRegularTint[0] > 0.99f
            && largeRegularTint[1] > 0.99f
            && largeRegularTint[2] > 0.99f,
        "regular glass resolves systemBackground for the current light appearance"
    );

    UIGlassEffect manual(UIGlassEffect::Style::regular);
    manual.setMaximumSampleRadius(48.0f);
    manual.setFrostRadius(7.0f);
    manual.setRefraction(0.21f);
    manual.setRefractionDepth(0.34f);
    manual.setDispersion(0.12f);
    manual.setSaturation(0.91f);
    manual.setContrast(0.83f);
    manual.setLuminosity(0.08f);
    manual.setTintColor(UIColor::systemBlue);
    manual.setTintOpacity(0.23f);
    manual.setEdgeStrength(0.72f);
    manual.setSpecularStrength(0.31f);
    manual.setDarkBackdropSpecularStrength(0.27f);
    manual.setBrightBackdropSpecularStrength(0.09f);
    manual.setLightAngle(30.0f);
    manual.setSpecularDirectionalPower(4.0f);
    manual.setSpecularWidth(6.0f);
    manual.setReflectionSampleDistance(9.0f);
    manual.setReflectionSpread(2.0f);
    manual.setBrightBackdropShade(0.004f);

    auto manualView = new_shared<BackdropEffectView>(manual);
    manualView->setFrame(NXRect(0, 0, 400, 400));
    const auto resolvedManual = manualView->effect();
    const auto resolvedManualTint = vectorUniform(resolvedManual, "tint", 4);
    const auto resolvedLightDirection = vectorUniform(
        resolvedManual,
        "specLightDirection",
        2
    );

    expect(
        std::abs(resolvedManual.maximumSampleRadius() - 48.0f) < 0.001f
            && std::abs(resolvedManual.backdropBlurRadius() - 7.0f) < 0.001f,
        "manual sample and frost radii override regular size adaptation"
    );
    expect(
        std::abs(scalarUniform(resolvedManual, "curve") - 0.21f) < 0.001f
            && std::abs(scalarUniform(resolvedManual, "refraction") - 0.34f) < 0.001f
            && std::abs(scalarUniform(resolvedManual, "dispersion") - 0.12f) < 0.001f,
        "manual refraction, depth, and dispersion reach the shader"
    );
    expect(
        std::abs(scalarUniform(resolvedManual, "saturation") - 0.91f) < 0.001f
            && std::abs(scalarUniform(resolvedManual, "contrast") - 0.83f) < 0.001f
            && std::abs(scalarUniform(resolvedManual, "luminosity") - 0.08f) < 0.001f,
        "manual color processing reaches the shader"
    );
    expect(
        std::abs(resolvedManualTint[0] - 0.0f) < 0.001f
            && std::abs(resolvedManualTint[1] - 122.0f / 255.0f) < 0.001f
            && std::abs(resolvedManualTint[2] - 1.0f) < 0.001f
            && std::abs(resolvedManualTint[3] - 59.0f / 255.0f) < 0.001f,
        "manual dynamic tint hue and opacity resolve into the shader"
    );
    expect(
        std::abs(resolvedLightDirection[0] - 0.8660254f) < 0.001f
            && std::abs(resolvedLightDirection[1] - 0.5f) < 0.001f,
        "manual light angle reaches the shader as a direction"
    );
    expect(
        std::abs(scalarUniform(resolvedManual, "edge") - 0.72f) < 0.001f
            && std::abs(scalarUniform(resolvedManual, "specStrength") - 0.31f) < 0.001f
            && std::abs(scalarUniform(resolvedManual, "specFallbackStrength") - 0.27f) < 0.001f
            && std::abs(scalarUniform(resolvedManual, "specBrightFallbackStrength") - 0.09f) < 0.001f
            && std::abs(scalarUniform(resolvedManual, "specDirectionalPower") - 4.0f) < 0.001f
            && std::abs(scalarUniform(resolvedManual, "specWidthPx") - 6.0f) < 0.001f,
        "manual rim and specular parameters reach the shader"
    );
    expect(
        std::abs(scalarUniform(resolvedManual, "reflectionSamplePx") - 9.0f) < 0.001f
            && std::abs(scalarUniform(resolvedManual, "reflectionSpreadPx") - 2.0f) < 0.001f
            && std::abs(scalarUniform(resolvedManual, "brightBackdropShade") - 0.004f) < 0.001f,
        "manual reflection and backdrop-shading parameters reach the shader"
    );

    bool rejectedInvalidOverride = false;
    try {
        manual.setTintOpacity(1.1f);
    } catch (const std::invalid_argument&) {
        rejectedInvalidOverride = true;
    }
    expect(
        rejectedInvalidOverride,
        "glass rejects an out-of-range manual parameter at its public boundary"
    );
}

GlassSamples renderGlassSamples(
    SkColor exteriorColor,
    SkColor interiorColor,
    NXFloat specFallbackStrength = 0.50f,
    NXFloat specBrightFallbackStrength = 0.14f
) {
    constexpr int padding = 24;
    constexpr int lensWidth = 40;
    constexpr int lensHeight = 24;
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(
        lensWidth + padding * 2,
        lensHeight + padding * 2
    ));
    if (!surface) return {};

    auto* canvas = surface->getCanvas();
    canvas->clear(exteriorColor);

    SkPaint interiorPaint;
    interiorPaint.setColor(interiorColor);
    canvas->drawRect(
        SkRect::MakeXYWH(padding, padding, lensWidth, lensHeight),
        interiorPaint
    );

    const auto glass = BackdropEffect::glass();
    BackdropEffect reflectionOnly(glass.shaderSource(), "content", 16, 0);
    reflectionOnly.setUniform("refraction", 0.0f);
    reflectionOnly.setUniform("curve", 0.0f);
    reflectionOnly.setUniform("dispersion", 0.0f);
    reflectionOnly.setUniform("saturation", 1.0f);
    reflectionOnly.setUniform("contrast", 1.0f);
    reflectionOnly.setUniform("luminosity", 0.0f);
    reflectionOnly.setUniform("tint", UIColor(255, 255, 255, 32));
    reflectionOnly.setUniform("edge", 1.0f);
    reflectionOnly.setUniform("specStrength", 0.52f);
    reflectionOnly.setUniform("specFallbackStrength", specFallbackStrength);
    reflectionOnly.setUniform("specBrightFallbackStrength", specBrightFallbackStrength);
    reflectionOnly.setUniform("specLightDirection", NXPoint(0.0f, -1.0f));
    reflectionOnly.setUniform("specDirectionalPower", 3.0f);
    reflectionOnly.setUniform("specWidthPx", 6.0f);
    reflectionOnly.setUniform("reflectionSamplePx", 10.0f);
    reflectionOnly.setUniform("reflectionSpreadPx", 0.0f);
    reflectionOnly.setUniform("brightBackdropShade", 0.008f);

    auto layer = new_shared<CABackdropEffectLayer>(reflectionOnly);
    layer->setBounds(NXRect(0, 0, lensWidth, lensHeight));
    layer->setCornerRadius(6);

    canvas->save();
    canvas->translate(padding, padding);
    layer->draw(canvas);
    canvas->restore();

    SkBitmap pixels;
    pixels.allocN32Pixels(surface->width(), surface->height());
    if (!surface->readPixels(pixels, 0, 0)) return {};
    return {
        .leftRim = pixels.getColor(padding + 1, padding + lensHeight / 2),
        .topRim = pixels.getColor(padding + lensWidth / 2, padding + 1),
        .bottomRim = pixels.getColor(
            padding + lensWidth / 2,
            padding + lensHeight - 2
        ),
        .center = pixels.getColor(
            padding + lensWidth / 2,
            padding + lensHeight / 2
        ),
    };
}

SkColor renderRadiusClampedReflectionSample() {
    constexpr int padding = 120;
    constexpr int lensWidth = 40;
    constexpr int lensHeight = 24;
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(
        lensWidth + padding * 2,
        lensHeight + padding * 2
    ));
    if (!surface) return SK_ColorTRANSPARENT;

    auto* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLUE);

    SkPaint nearExteriorPaint;
    nearExteriorPaint.setColor(SK_ColorRED);
    canvas->drawRect(
        SkRect::MakeXYWH(padding - 18, padding, 18, lensHeight),
        nearExteriorPaint
    );

    SkPaint interiorPaint;
    interiorPaint.setColor(SkColorSetRGB(48, 48, 48));
    canvas->drawRect(
        SkRect::MakeXYWH(padding, padding, lensWidth, lensHeight),
        interiorPaint
    );

    const auto glass = BackdropEffect::glass();
    BackdropEffect reflectionOnly(glass.shaderSource(), "content", 16, 0);
    reflectionOnly.setUniform("refraction", 0.0f);
    reflectionOnly.setUniform("curve", 0.0f);
    reflectionOnly.setUniform("dispersion", 0.0f);
    reflectionOnly.setUniform("saturation", 1.0f);
    reflectionOnly.setUniform("contrast", 1.0f);
    reflectionOnly.setUniform("luminosity", 0.0f);
    reflectionOnly.setUniform("tint", UIColor(0, 0, 0, 0));
    reflectionOnly.setUniform("edge", 1.0f);
    reflectionOnly.setUniform("specStrength", 1.0f);
    reflectionOnly.setUniform("specFallbackStrength", 0.0f);
    reflectionOnly.setUniform("specBrightFallbackStrength", 0.0f);
    reflectionOnly.setUniform("specLightDirection", NXPoint(0.0f, -1.0f));
    reflectionOnly.setUniform("specDirectionalPower", 3.0f);
    reflectionOnly.setUniform("specWidthPx", 6.0f);
    // Deliberately request a coordinate far beyond the declared filter input.
    // The automatic maximumSampleRadius uniform must clamp it to nearby red.
    reflectionOnly.setUniform("reflectionSamplePx", 100.0f);
    reflectionOnly.setUniform("reflectionSpreadPx", 0.0f);
    reflectionOnly.setUniform("brightBackdropShade", 0.0f);

    auto layer = new_shared<CABackdropEffectLayer>(reflectionOnly);
    layer->setBounds(NXRect(0, 0, lensWidth, lensHeight));
    layer->setCornerRadius(6);

    canvas->save();
    canvas->translate(padding, padding);
    layer->draw(canvas);
    canvas->restore();

    SkBitmap pixels;
    pixels.allocN32Pixels(surface->width(), surface->height());
    if (!surface->readPixels(pixels, 0, 0)) return SK_ColorTRANSPARENT;
    return pixels.getColor(padding + 1, padding + lensHeight / 2);
}

SkColor renderDefaultGlassRefractionSample() {
    constexpr int padding = 24;
    constexpr int lensWidth = 120;
    constexpr int lensHeight = 60;
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(
        lensWidth + padding * 2,
        lensHeight + padding * 2
    ));
    if (!surface) return SK_ColorTRANSPARENT;

    auto* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLUE);

    // The inspected point is red in the undistorted source. Strong default
    // refraction bends it downward across this boundary into the blue region.
    SkPaint redPaint;
    redPaint.setColor(SK_ColorRED);
    canvas->drawRect(
        SkRect::MakeXYWH(0, 0, surface->width(), padding + 10),
        redPaint
    );

    auto layer = new_shared<CABackdropEffectLayer>(BackdropEffect::glass());
    layer->setBounds(NXRect(0, 0, lensWidth, lensHeight));
    layer->setCornerRadius(lensHeight * 0.5f);

    canvas->save();
    canvas->translate(padding, padding);
    layer->draw(canvas);
    canvas->restore();

    SkBitmap pixels;
    pixels.allocN32Pixels(surface->width(), surface->height());
    if (!surface->readPixels(pixels, 0, 0)) return SK_ColorTRANSPARENT;
    return pixels.getColor(padding + lensWidth / 2, padding + 6);
}

struct RefractionSeamSamples {
    SkColor above = SK_ColorTRANSPARENT;
    SkColor below = SK_ColorTRANSPARENT;
};

RefractionSeamSamples renderLargeGlassDiagonalSeamSamples() {
    constexpr int lensWidth = 340;
    constexpr int lensHeight = 140;
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(
        lensWidth,
        lensHeight
    ));
    if (!surface) return {};

    auto* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLUE);

    // Both inspected pixels are red without displacement. An uncapped deep
    // refraction field sends the pixel above the rounded-rect medial axis to
    // the blue side while the pixel below it remains red, revealing a seam.
    SkPaint redPaint;
    redPaint.setColor(SK_ColorRED);
    canvas->drawRect(
        SkRect::MakeXYWH(0, 0, 42, surface->height()),
        redPaint
    );

    const auto glass = BackdropEffect::glass();
    BackdropEffect refractionOnly(glass.shaderSource(), "content", 64, 0);
    refractionOnly.setUniform("refraction", 0.90f);
    refractionOnly.setUniform("curve", 0.55f);
    refractionOnly.setUniform("dispersion", 0.0f);
    refractionOnly.setUniform("saturation", 1.0f);
    refractionOnly.setUniform("contrast", 1.0f);
    refractionOnly.setUniform("luminosity", 0.0f);
    refractionOnly.setUniform("tint", UIColor(0, 0, 0, 0));
    refractionOnly.setUniform("edge", 0.0f);
    refractionOnly.setUniform("specStrength", 0.0f);
    refractionOnly.setUniform("brightBackdropShade", 0.0f);

    auto layer = new_shared<CABackdropEffectLayer>(refractionOnly);
    layer->setBounds(NXRect(0, 0, lensWidth, lensHeight));
    layer->setCornerRadius(36);

    layer->draw(canvas);

    SkBitmap pixels;
    pixels.allocN32Pixels(surface->width(), surface->height());
    if (!surface->readPixels(pixels, 0, 0)) return {};
    return {
        .above = pixels.getColor(39, 99),
        .below = pixels.getColor(39, 101),
    };
}

void testGlassSpecularReflectionUsesBackdropOverscan() {
    const auto interior = SkColorSetRGB(48, 48, 48);
    const auto red = renderGlassSamples(SkColorSetRGB(240, 32, 24), interior);
    const auto blue = renderGlassSamples(SkColorSetRGB(24, 48, 240), interior);
    const auto blackWithoutFallback = renderGlassSamples(
        SK_ColorBLACK,
        SK_ColorBLACK,
        0
    );
    const auto blackWithFallback = renderGlassSamples(SK_ColorBLACK, SK_ColorBLACK);
    const auto white = renderGlassSamples(SK_ColorWHITE, SK_ColorWHITE);

    expect(
        SkColorGetR(red.leftRim) > SkColorGetB(red.leftRim) + 30,
        "red content outside the lens colors its specular rim"
    );
    expect(
        SkColorGetB(blue.leftRim) > SkColorGetR(blue.leftRim) + 30,
        "blue content outside the lens colors its specular rim"
    );
    expect(
        brightness(blackWithFallback.topRim)
            > brightness(blackWithoutFallback.topRim) + 30,
        "pure black exterior content receives the neutral specular fallback"
    );
    expect(
        std::abs(
            static_cast<int>(SkColorGetR(blackWithFallback.topRim))
            - static_cast<int>(SkColorGetB(blackWithFallback.topRim))
        ) <= 2,
        "the pure-black specular fallback remains neutral white"
    );
    expect(
        brightness(blackWithFallback.topRim)
            > brightness(blackWithFallback.leftRim) + 30,
        "the black-backdrop fallback forms a partial highlight, not a full border"
    );
    expect(
        brightness(blackWithFallback.bottomRim)
            > brightness(blackWithFallback.leftRim) + 30,
        "the black-backdrop fallback retains the opposite optical highlight"
    );
    expect(
        brightness(white.topRim) < brightness(white.center) - 15,
        "bright neutral content receives a contrasting specular reflection"
    );
    expect(
        brightness(white.leftRim) < brightness(white.center),
        "bright neutral content keeps a subtle ambient side reflection"
    );
    expect(
        brightness(white.center) < brightness(SK_ColorWHITE),
        "the glass body remains faintly visible on a pure-white backdrop"
    );
}

void testGlassBackdropReadsStayInsideDeclaredRadius() {
    const auto rim = renderRadiusClampedReflectionSample();
    expect(
        SkColorGetR(rim) > SkColorGetB(rim) + 30,
        "glass clamps retuned reflection reads to its declared backdrop radius"
    );

    auto glass = BackdropEffect::glass();
    bool rejectedAutomaticUniform = false;
    try {
        glass.setUniform("maximumSampleRadius", 8.0f);
    } catch (const std::invalid_argument&) {
        rejectedAutomaticUniform = true;
    }
    expect(
        rejectedAutomaticUniform,
        "maximumSampleRadius is populated automatically and cannot be overridden"
    );
}

void testDefaultGlassRefractionExtendsIntoLensBody() {
    const auto refracted = renderDefaultGlassRefractionSample();
    expect(
        SkColorGetB(refracted) > SkColorGetR(refracted) + 30,
        "default glass strongly refracts source content beyond the outer edge band"
    );
}

void testLargeGlassRefractionStopsBeforeDiagonalMedialAxis() {
    const auto samples = renderLargeGlassDiagonalSeamSamples();
    expect(
        SkColorGetR(samples.above) > SkColorGetB(samples.above) + 30
            && SkColorGetR(samples.below) > SkColorGetB(samples.below) + 30,
        "large rounded glass fades refraction before its diagonal normal seam"
    );
}

} // namespace

int main() {
    testMaskLayersUseLocalBounds();
    testOpacityLayersRetainBackdropForEffects();
    testBackdropFilterCacheTracksGeometry();
    testGlassStylesResolveThroughTheirView();
    testGlassSpecularReflectionUsesBackdropOverscan();
    testGlassBackdropReadsStayInsideDeclaredRadius();
    testDefaultGlassRefractionExtendsIntoLensBody();
    testLargeGlassRefractionStopsBeforeDiagonalMedialAxis();

    if (failures != 0) {
        std::cerr << failures << " layer-rendering assertion(s) failed\n";
        return 1;
    }
    std::cout << "CALayer rendering tests passed\n";
    return 0;
}
