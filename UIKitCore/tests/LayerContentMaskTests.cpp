#include <CALayer.h>
#include <CABlurLayer.h>
#include "LayerContentMask.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"

#include <cmath>
#include <iostream>

using namespace NXKit;
using namespace NXKit::Internal;

namespace {
int failures = 0;
void expect(bool condition, const char* message) {
    if (!condition) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
bool near(int value, int expected) { return std::abs(value - expected) <= 1; }
class HalfContentMask final : public LayerContentMask {
public:
    bool enabled = true;
    mutable int draws = 0;
    bool active(const CALayer&) const override { return enabled; }
    void draw(SkCanvas* canvas, const CALayer& layer) const override {
        ++draws;
        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        paint.setAlpha(128);
        canvas->drawRect(SkRect::MakeWH(layer.bounds().width(), layer.bounds().height()), paint);
    }
};
class AnalyticContentMask final : public LayerContentMask {
public:
    mutable int fallbackDraws=0;
    bool active(const CALayer&) const override { return true; }
    sk_sp<SkShader> contentShader(const CALayer&) const override {
        return SkShaders::Color(SkColorSetARGB(128,255,255,255));
    }
    void draw(SkCanvas*,const CALayer&) const override { ++fallbackDraws; }
};
std::shared_ptr<CALayer> layer(const NXRect& frame, const std::optional<UIColor>& background = std::nullopt) {
    auto result = new_shared<CALayer>();
    result->setFrame(frame);
    result->setBackgroundColor(background);
    return result;
}
SkBitmap render(const std::shared_ptr<CALayer>& root, int scale = 1) {
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(8 * scale, 8 * scale));
    auto* canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->scale(scale, scale);
    const auto saveCount = canvas->getSaveCount();
    root->skiaRender(canvas);
    expect(canvas->getSaveCount() == saveCount, "content mask balances the canvas stack");
    SkBitmap pixels;
    pixels.allocN32Pixels(8 * scale, 8 * scale);
    surface->readPixels(pixels, 0, 0);
    return pixels;
}

void childAlphaAndOwnership() {
    for (int scale : {1, 2, 3}) {
        auto root = layer({0, 0, 8, 8});
        root->setBounds({4, 6, 8, 8});
        auto child = layer({4, 6, 8, 8}, UIColor::red);
        root->addSublayer(child);
        auto hook = std::make_shared<HalfContentMask>();
        LayerContentMaskAccess::add(root, hook);
        const auto pixels = render(root, scale);
        expect(near(SkColorGetA(pixels.getColor(2 * scale, 2 * scale)), 128)
               && SkColorGetR(pixels.getColor(2 * scale, 2 * scale)) == 255,
               "content hook attenuates actual child pixels at every backing scale and scroll offset");
        hook->enabled = false;
        expect(SkColorGetA(render(root, scale).getColor(2 * scale, 2 * scale)) == 255 && hook->draws == 1,
               "inactive hooks leave content intact without drawing");
        hook->enabled = true;
        auto presentation = root->createPresentation();
        expect(near(SkColorGetA(render(presentation, scale).getColor(2 * scale, 2 * scale)), 128),
               "presentation copies preserve active weak content hooks");
        std::weak_ptr<HalfContentMask> weak = hook;
        hook.reset();
        expect(weak.expired(), "model and presentation layers do not own content hooks");
        expect(SkColorGetA(render(root, scale).getColor(2 * scale, 2 * scale)) == 255
               && SkColorGetA(render(presentation, scale).getColor(2 * scale, 2 * scale)) == 255,
               "expired hooks stop affecting both model and presentation output");
    }
}

void analyticMaskSkipsPictureFallback() {
    for(int scale:{1,2,3}) {
        auto root=layer({0,0,8,8});root->addSublayer(layer({0,0,8,8},UIColor::red));
        auto mask=std::make_shared<AnalyticContentMask>();LayerContentMaskAccess::add(root,mask);
        const auto pixel=render(root,scale).getColor(2*scale,2*scale);
        expect(near(SkColorGetA(pixel),128)&&SkColorGetR(pixel)==255,
               "analytic content shaders preserve group alpha at every display scale");
        expect(mask->fallbackDraws==0,"analytic content shaders bypass mask-picture recording");
    }
}

void backgroundAndApplicationMask() {
    auto root = layer({0, 0, 8, 8}, UIColor::blue);
    root->addSublayer(layer({0, 0, 4, 8}, UIColor::red));
    auto hook = std::make_shared<HalfContentMask>();
    LayerContentMaskAccess::add(root, hook);
    auto pixels = render(root);
    const auto mixed = pixels.getColor(2, 2);
    expect(SkColorGetA(mixed) == 255 && near(SkColorGetR(mixed), 128) && near(SkColorGetB(mixed), 127),
           "fading children reveals the parent's background");
    expect(pixels.getColor(6, 2) == SK_ColorBLUE, "content mask preserves uncovered opaque parent background");

    auto applicationMask = layer({0, 0, 8, 4}, UIColor::white);
    root->setMask(applicationMask);
    pixels = render(root);
    expect(root->mask() == applicationMask, "content hook does not replace the application's CALayer mask");
    expect(SkColorGetA(pixels.getColor(2, 6)) == 0 && SkColorGetA(pixels.getColor(6, 6)) == 0,
           "application mask still clips both background and faded children");
    expect(near(SkColorGetR(pixels.getColor(2, 2)), 128) && pixels.getColor(6, 2) == SK_ColorBLUE,
           "application and content masks compose independently in retained pixels");
    auto presentation = root->createPresentation();
    expect(presentation->mask() == applicationMask && SkColorGetA(render(presentation).getColor(2, 6)) == 0,
           "presentation copy retains the application mask alongside content hooks");

    auto translucent = layer({0, 0, 8, 8}, UIColor(0, 0, 255, 128));
    LayerContentMaskAccess::add(translucent, hook);
    expect(near(SkColorGetA(render(translucent).getColor(2, 2)), 128),
           "content-only mask preserves translucent parent background alpha when there are no children");
    translucent->addSublayer(layer({0, 0, 4, 8}, UIColor::red));
    pixels = render(translucent);
    expect(near(SkColorGetA(pixels.getColor(6, 2)), 128),
           "content-only mask preserves uncovered translucent background alongside visible children");
    expect(near(SkColorGetA(pixels.getColor(2, 2)), 192),
           "faded child composes over the original translucent parent background");
}

void descendantBackdrop() {
    auto root = layer({0, 0, 8, 8}, UIColor::red);
    root->addSublayer(layer({4, 0, 4, 8}, UIColor::blue));
    auto content = layer({0, 0, 8, 8});
    auto blur = new_shared<CABlurLayer>();
    blur->setFrame({0, 0, 8, 8});
    blur->setBlurRadius(1.5f);
    content->addSublayer(blur);
    root->addSublayer(content);
    auto hook = std::make_shared<HalfContentMask>();
    hook->enabled = false;
    LayerContentMaskAccess::add(content, hook);
    const auto baseline = render(root).getColor(3, 4);
    expect(SkColorGetB(baseline) > 20, "unmasked descendant blur samples the external blue backdrop");
    hook->enabled = true;
    const auto faded = render(root).getColor(3, 4);
    expect(std::abs(int(SkColorGetB(faded)) - int(SkColorGetB(baseline)) / 2) <= 3,
           "masked descendant blur retains its attenuated external backdrop contribution");
}

void overlappingChildren() {
    auto root = layer({0, 0, 8, 8});
    root->addSublayer(layer({0, 0, 6, 8}, UIColor(255, 0, 0, 128)));
    root->addSublayer(layer({2, 0, 6, 8}, UIColor(0, 0, 255, 128)));
    auto hook = std::make_shared<HalfContentMask>();
    hook->enabled = false;
    LayerContentMaskAccess::add(root, hook);
    auto pixels = render(root);
    expect(near(SkColorGetA(pixels.getColor(3, 4)), 192),
           "overlapping translucent children first composite at their original alpha");
    hook->enabled = true;
    pixels = render(root);
    expect(near(SkColorGetA(pixels.getColor(3, 4)), 96)
           && near(SkColorGetA(pixels.getColor(1, 4)), 64)
           && near(SkColorGetA(pixels.getColor(7, 4)), 64),
           "content mask applies once to the combined child result rather than to each child or twice per group");
    auto second = std::make_shared<HalfContentMask>();
    LayerContentMaskAccess::add(root, second);
    pixels = render(root);
    expect(near(SkColorGetA(pixels.getColor(3, 4)), 48),
           "multiple content mask hooks combine coverage once each");
}
void subtreeSelectionAndBackdrop() {
    class SelectionMask final:public LayerContentMask {
    public:
        std::weak_ptr<CALayer> selected;
        bool blackBackdrop=false;
        bool affectsSubtree() const override{return true;}
        bool active(const CALayer& value) const override{return selected.lock().get()==&value;}
        void drawBackdrop(SkCanvas* canvas,const CALayer&) const override {
            if(blackBackdrop)canvas->drawColor(SK_ColorBLACK);
        }
        void draw(SkCanvas* canvas,const CALayer&) const override {
            SkPaint paint;paint.setColor(SkColorSetARGB(128,255,255,255));
            canvas->drawRect(SkRect::MakeWH(8,8),paint);
        }
    };
    auto root=layer({0,0,8,8},UIColor::white);
    auto first=layer({0,0,4,8});first->addSublayer(layer({0,0,4,8},UIColor::red));
    auto second=layer({4,0,4,8});second->addSublayer(layer({0,0,4,8},UIColor::red));
    root->addSublayer(first);root->addSublayer(second);
    auto hook=std::make_shared<SelectionMask>();hook->selected=first;
    LayerContentMaskAccess::add(root,hook);
    auto pixels=render(root);
    expect(near(SkColorGetG(pixels.getColor(1,4)),127)&&SkColorGetG(pixels.getColor(6,4))==0,
           "subtree providers affect only their selected descendant's content");
    hook->selected=second;hook->blackBackdrop=true;pixels=render(root);
    expect(SkColorGetR(pixels.getColor(1,4))==255&&near(SkColorGetR(pixels.getColor(6,4)),128)
           &&SkColorGetG(pixels.getColor(6,4))==0&&SkColorGetA(pixels.getColor(6,4))==255,
           "live subtree selection changes without reattaching and composites over its optional backdrop");
    second->removeFromSuperlayer();pixels=render(root);
    expect(SkColorGetR(pixels.getColor(1,4))==255&&SkColorGetG(pixels.getColor(6,4))==255,
           "removed descendants leave no inherited provider or backdrop behind");
}
}

int main() {
    childAlphaAndOwnership();
    analyticMaskSkipsPictureFallback();
    backgroundAndApplicationMask();
    descendantBackdrop();
    overlappingChildren();
    subtreeSelectionAndBackdrop();
    if (failures) return 1;
    std::cout << "Layer content mask alpha, background, ownership, and presentation tests passed\n";
    return 0;
}
