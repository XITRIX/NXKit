#include <BackdropEffect.h>
#include <CALayer.h>

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
    testBackdropFilterCacheTracksGeometry();
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
