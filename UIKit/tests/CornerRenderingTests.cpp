#include <BackdropEffectView.h>
#include <CABlurLayer.h>
#include <CALayer.h>
#include "LayerCornerGeometry.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

using namespace NXKit;
namespace {
int failures = 0;
void expect(bool value, const std::string& message) {
    if (value) return;
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}
void save(const SkBitmap& pixels, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    file << "P6\n" << pixels.width() << ' ' << pixels.height() << "\n255\n";
    for (int y = 0; y < pixels.height(); ++y) for (int x = 0; x < pixels.width(); ++x) {
        const auto color = pixels.getColor(x, y);
        const unsigned char rgb[] = {static_cast<unsigned char>(SkColorGetR(color)),
            static_cast<unsigned char>(SkColorGetG(color)), static_cast<unsigned char>(SkColorGetB(color))};
        file.write(reinterpret_cast<const char*>(rgb), 3);
    }
}
SkBitmap render(bool glass, CALayerCornerCurve curve, int scale, bool clip, bool rotated = false) {
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(220 * scale, 180 * scale));
    auto* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLACK);
    canvas->scale(scale, scale);
    if (rotated) {
        canvas->translate(110, 90);
        canvas->rotate(17);
        canvas->scale(1.06f, 0.94f);
        canvas->translate(-110, -90);
    }
    std::shared_ptr<CALayer> layer;
    if (glass) {
        UIGlassEffect effect;
        effect.setTintColor(UIColor::white);
        effect.setTintOpacity(1);
        effect.setContrast(0);
        effect.setLuminosity(0);
        effect.setEdgeStrength(0);
        effect.setEdgeAbsorption(0);
        effect.setBrightBackdropShade(0);
        effect.setRefraction(0);
        effect.setFrostRadius(0);
        auto view = new_shared<BackdropEffectView>(effect);
        layer = view->layer();
        // Retain the delegate for the duration of the draw.
        view->setFrame({29.25f, 24.25f, 160, 128});
        layer->setCornerRadius(40);
        layer->setCornerCurve(curve);
        layer->setContentsScale(scale);
        layer->setShadowOpacity(0);
        layer->setMasksToBounds(clip);
        layer->skiaRender(canvas);
    } else {
        layer = new_shared<CALayer>();
        layer->setFrame({29.25f, 24.25f, 160, 128});
        layer->setBackgroundColor(UIColor::white);
        layer->setCornerRadius(40);
        layer->setCornerCurve(curve);
        layer->setContentsScale(scale);
        layer->setMasksToBounds(clip);
        layer->skiaRender(canvas);
    }
    SkBitmap pixels;
    pixels.allocN32Pixels(surface->width(), surface->height());
    expect(surface->readPixels(pixels, 0, 0), "corner fixture has readable pixels");
    return pixels;
}
void testAPI() {
    auto layer = new_shared<CALayer>();
    expect(layer->cornerCurve() == CALayerCornerCurve::continuous, "continuous corners are the NXKit default");
    layer->setCornerCurve(CALayerCornerCurve::circular);
    expect(layer->copy()->cornerCurve() == CALayerCornerCurve::circular,
           "layer copies retain an explicitly circular curve");
    auto animation = new_shared<CABasicAnimation>("bounds");
    animation->duration = 10;
    animation->fromValue = NXRect(0, 0, 100, 80);
    animation->toValue = NXRect(0, 0, 160, 128);
    layer->add(animation, "bounds");
    layer->animateAt(Timer(10));
    layer->setCornerCurve(CALayerCornerCurve::continuous);
    expect(layer->presentation()->cornerCurve() == CALayerCornerCurve::continuous,
           "changing the curve updates an already active presentation layer");
    layer->removeAllAnimations();
    bool rejected = false;
    try { layer->setCornerCurve(static_cast<CALayerCornerCurve>(99)); }
    catch (const std::invalid_argument&) { rejected = true; }
    expect(rejected, "invalid corner curve values are rejected");
    auto view = new_shared<UIView>();
    expect(view->applyXMLAttribute("cornerCurve", "circular")
               && view->layer()->cornerCurve() == CALayerCornerCurve::circular
               && view->applyXMLAttribute("cornerCurve", "continuous")
               && !view->applyXMLAttribute("cornerCurve", "invalid"),
           "XML exposes both curves and rejects unknown names");
    view->drawAndLayoutTreeIfNeeded();
    expect(!view->layer()->needsDisplay(), "the display pass clears the layer's dirty state");
    view->layer()->setCornerCurve(CALayerCornerCurve::circular);
    expect(view->layer()->needsDisplay(), "changing cornerCurve invalidates rendering");
    auto blur = new_shared<CABlurLayer>();
    blur->setCornerCurve(CALayerCornerCurve::circular);
    expect(blur->copy()->cornerCurve() == CALayerCornerCurve::circular,
           "blur presentation copies preserve the selected curve");
    expect(Internal::layerCornerPath({0, 0}, 40, CALayerCornerCurve::continuous).isEmpty(),
           "empty bounds produce an empty shape");
    expect(Internal::layerCornerDistance({0, 0}, {0, 0}, 40, CALayerCornerCurve::continuous) > 0,
           "empty glass cannot pass the material hit check");
    expect(Internal::layerCornerPath({100, 80}, std::numeric_limits<float>::infinity(), CALayerCornerCurve::continuous)
               == Internal::layerCornerPath({100, 80}, 0, CALayerCornerCurve::continuous),
           "nonfinite radii cannot produce nonfinite render paths");
    expect(Internal::layerCornerPath({100, 80}, 400, CALayerCornerCurve::continuous)
               == Internal::layerCornerPath({100, 80}, 40, CALayerCornerCurve::continuous),
           "oversized radii clamp to the shortest half dimension");
}
// Independent 50%-coverage contour coordinates from the retained UIKit
// CALayer fixture (2x iPadOS 26.4.1, continuous column, 300pt width).
// In particular the shallow samples reject the former near-circular cap.
void testNativeCapsuleTransitions() {
    constexpr float depths[]={.25f,1.25f,3.25f,8.25f,15.25f,25.25f};
    constexpr float edges[][6]={
        {61.91667f,43.78704f,33.65517f,22.545f,14.05f,6.55675f},
        {105.95f,77.86765f,61.98171f,45.36039f,33.04907f,21.67045f}
    };
    for(int row=0;row<2;++row) {
        const float height=row?160:100;
        const NXSize size{300,height};
        const auto path=Internal::layerCornerPath(size,height*.5f,CALayerCornerCurve::continuous);
        for(int i=0;i<6;++i) {
            float lo=0,hi=150;
            for(int j=0;j<24;++j) {
                const auto x=(lo+hi)*.5f;
                if(Internal::layerCornerDistance({x-150,depths[i]-height*.5f},size,height*.5f,CALayerCornerCurve::continuous)>0)lo=x;
                else hi=x;
            }
            // At the nearly flat tangent, 2x coverage quantization causes a
            // large horizontal uncertainty. Score normal distance there;
            // deeper samples also constrain the horizontal coordinate.
            const auto nativeDistance=Internal::layerCornerDistance(
                {edges[row][i]-150,depths[i]-height*.5f},size,height*.5f,CALayerCornerCurve::continuous);
            expect(std::abs(nativeDistance)<.3f && (i==0 || std::abs((lo+hi)*.5f-edges[row][i])<.65f),
                   "continuous capsule transition follows the native contour");
            expect(path.contains(hi+.15f,depths[i]) && !path.contains(lo-.15f,depths[i]),
                   "render path and optical distance agree at the eased transition");
        }
    }
    for(NXSize size : {NXSize{300.5f,100.25f},NXSize{100.25f,300.5f},NXSize{80,80}}) {
        for(float radius : {.001f,12.f,24.f,std::min(size.width,size.height)*.5f}) {
            const auto path=Internal::layerCornerPath(size,radius,CALayerCornerCurve::continuous);
            for(int y=0;y<=20;++y) for(int x=0;x<=20;++x) {
                NXPoint p{size.width*(x/20.f-.5f),size.height*(y/20.f-.5f)};
                const auto d=Internal::layerCornerDistance(p,size,radius,CALayerCornerCurve::continuous);
                const auto transpose=Internal::layerCornerDistance({p.y,p.x},{size.height,size.width},radius,CALayerCornerCurve::continuous);
                expect(std::isfinite(d)&&std::abs(d-transpose)<.002f,
                       "continuous distances remain finite and transpose symmetrically");
                if(std::abs(d)>.2f)
                    expect(path.contains(p.x+size.width*.5f,p.y+size.height*.5f)==(d<0),
                           "path and optical field classify tall, wide and tiny-radius bounds alike");
            }
        }
    }
}
void testBorder() {
    for (bool clip : {false, true}) {
        auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(160, 120));
        auto* canvas = surface->getCanvas();
        canvas->clear(SK_ColorBLACK);
        auto layer = new_shared<CALayer>();
        layer->setFrame({20, 20, 100, 80});
        layer->setCornerRadius(20);
        layer->setBackgroundColor(UIColor::white);
        layer->setBorderColor(UIColor(255, 0, 0));
        layer->setBorderWidth(3);
        layer->setMasksToBounds(clip);
        layer->skiaRender(canvas);
        SkBitmap pixels;
        pixels.allocN32Pixels(160, 120);
        surface->readPixels(pixels, 0, 0);
        expect(pixels.getColor(19, 60) == SK_ColorBLACK
                   && pixels.getColor(20, 20) == SK_ColorBLACK,
               "the corner border stays inside the selected silhouette and layer bounds");
        expect(pixels.getColor(21, 60) == SK_ColorRED && pixels.getColor(24, 60) == SK_ColorWHITE,
               "borderWidth measures the visible inside stroke");
    }
}
void testCoverage() {
    for (const auto curve : {CALayerCornerCurve::circular, CALayerCornerCurve::continuous}) {
        for (int scale : {1, 2, 3}) for (bool clipped : {false, true}) for (bool rotated : {false, true}) {
            const auto solid = render(false, curve, scale, clipped, rotated);
            const auto glass = render(true, curve, scale, clipped, rotated);
            int antialiased = 0, mismatched = 0, maximum = 0;
            for (int y = 0; y < solid.height(); ++y) for (int x = 0; x < solid.width(); ++x) {
                const auto expected = SkColorGetR(solid.getColor(x, y));
                const auto actual = SkColorGetR(glass.getColor(x, y));
                if (expected > 2 && expected < 253) ++antialiased;
                const auto difference = std::abs(static_cast<int>(actual) - static_cast<int>(expected));
                maximum = std::max(maximum, difference);
                // Skia's path rasterizer and AA clip mask quantize coverage
                // differently (up to eight levels); a repeated AA edge loses
                // roughly 50–90 levels and must still fail this comparison.
                if (difference > 8) ++mismatched;
            }
            expect(antialiased > 50, "curved boundaries have fractional device-pixel coverage");
            if (mismatched) std::cerr << "Coverage: scale=" << scale << " clip=" << clipped
                << " rotate=" << rotated << " curve=" << static_cast<int>(curve)
                << " mismatches=" << mismatched << " maximum=" << maximum << '\n';
            expect(mismatched == 0, "glass has the same single AA edge coverage as the solid layer");
        }
    }
    const auto circular = render(false, CALayerCornerCurve::circular, 2, false);
    const auto continuous = render(false, CALayerCornerCurve::continuous, 2, false);
    int changed = 0;
    for (int y = 0; y < circular.height(); ++y) for (int x = 0; x < circular.width(); ++x) {
        if (circular.getColor(x, y) != continuous.getColor(x, y)) ++changed;
    }
    expect(changed > 200, "continuous corners visibly differ from circular arcs at the same radius");
}
void reference(const std::string& output) {
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1440, 1520));
    auto* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLACK);
    canvas->scale(2, 2);
    const int ys[] = {20, 140, 260, 440, 560}, heights[] = {100, 100, 160, 100, 160}, radii[] = {12, 24, 52, 50, 80};
    for (int row = 0; row < 5; ++row) for (int column = 0; column < 2; ++column) {
        auto layer = new_shared<CALayer>();
        layer->setFrame({static_cast<float>(20 + column * 360), static_cast<float>(ys[row]), 300,
                         static_cast<float>(heights[row])});
        layer->setCornerRadius(radii[row]);
        layer->setCornerCurve(column ? CALayerCornerCurve::continuous : CALayerCornerCurve::circular);
        layer->setBackgroundColor(UIColor::white);
        layer->skiaRender(canvas);
    }
    SkBitmap pixels;
    pixels.allocN32Pixels(1440, 1520);
    surface->readPixels(pixels, 0, 0);
    save(pixels, output);
}
}
int main(int argc, char** argv) {
    testAPI();
    testNativeCapsuleTransitions();
    testCoverage();
    testBorder();
    if (argc > 1) reference(argv[1]);
    if (failures) return 1;
    std::cout << "Corner rendering tests passed\n";
}
