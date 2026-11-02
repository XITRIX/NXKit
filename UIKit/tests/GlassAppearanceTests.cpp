#include <UIGlassEffect.h>
#include <BackdropEffectView.h>
#include <DispatchQueue.h>
#include <CABasicAnimation.h>
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include <cmath>
#include <iostream>
#include <string>

using namespace NXKit;

namespace NXKit {
class GlassAppearanceTestHarness {
public:
    static std::shared_ptr<UITraitCollection> traits(UIUserInterfaceStyle style, NXFloat scale = 2) {
        auto result = std::make_shared<UITraitCollection>();
        result->_userInterfaceStyle = style;
        result->_displayScale = scale;
        return result;
    }
};
class DispatchQueueTestHarness {
public:
    static void pumpMain() { DispatchQueue::main()->performAll(); }
};
}

namespace {
int failures = 0;
void expect(bool condition, const std::string& message) {
    if (condition) return;
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}
class TraitRoot final: public UIView {
public:
    explicit TraitRoot(UIUserInterfaceStyle style) {
        _traitCollection = GlassAppearanceTestHarness::traits(style);
        setFrame({0, 0, 480, 320});
    }
    void setDisplayScale(NXFloat scale) {
        const auto previous = _traitCollection;
        _traitCollection = GlassAppearanceTestHarness::traits(previous->userInterfaceStyle(), scale);
        traitCollectionDidChange(previous);
    }
};

void testBackingScale() {
    auto root = new_shared<TraitRoot>(UIUserInterfaceStyle::light);
    auto view = new_shared<BackdropEffectView>();
    auto custom = new_shared<BackdropEffectView>(UIGlassEffect(UIGlassEffect::Style::clear).backdropEffect());
    root->addSubview(view); root->addSubview(custom);
    expect(view->layer()->contentsScale() == 2 && custom->layer()->contentsScale() == 2,
        "glass and custom backdrop filters inherit Retina backing resolution");
    auto animation = std::make_shared<CABasicAnimation>("bounds");
    animation->duration = 1; animation->fromValue = NXRect(0,0,52,52); animation->toValue = NXRect(0,0,80,80);
    view->layer()->add(animation,"bounds");
    root->setDisplayScale(3);
    expect(view->layer()->contentsScale() == 3 && view->layer()->presentationOrSelf()->contentsScale() == 3,
        "display-scale changes update active presentation filters immediately");
    expect(custom->layer()->contentsScale() == 3, "custom backdrop resolution follows screen changes");
    view->layer()->setContentsScale(1.5f);
    root->setDisplayScale(3);
    view->setFrame({0,0,52,52});
    expect(view->layer()->contentsScale() == 1.5f, "same-screen traits and layout retain an explicit layer scale override");
    root->setDisplayScale(0);
    expect(view->layer()->contentsScale() == 1, "invalid screen scale resolves to a finite one-point filter");
    auto other = new_shared<TraitRoot>(UIUserInterfaceStyle::dark);
    other->addSubview(view);
    expect(view->layer()->contentsScale() == 2, "reparenting adopts the new screen's filter resolution");
}

void testOptInGlassAppearance() {
    using Style = UIUserInterfaceStyle;
    auto root = new_shared<TraitRoot>(Style::light);
    UIGlassEffect effect;
    auto view = new_shared<BackdropEffectView>(effect);
    view->setFrame({10, 10, 120, 64});
    auto ink = new_shared<UIView>(NXRect(20, 20, 8, 8));
    ink->setBackgroundColor(UIColor::label);
    view->addSubview(ink);
    root->addSubview(view);
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(480, 320));
    const auto draw = [&](SkColor background, bool pump = true) {
        surface->getCanvas()->clear(background);
        root->layer()->skiaRender(surface->getCanvas());
        if (pump) DispatchQueueTestHarness::pumpMain();
    };
    draw(SK_ColorBLACK);
    expect(ink->traitCollection()->userInterfaceStyle() == Style::light,
           "default glass leaves content appearance inherited over black");

    effect.setAdaptsAppearanceToBackdrop(true);
    effect.setRefraction(0.23f);
    view->setEffect(effect);
    draw(SK_ColorBLACK);
    expect(view->traitCollection()->userInterfaceStyle() == Style::light
               && ink->traitCollection()->userInterfaceStyle() == Style::dark,
           "opt-in small glass changes content appearance without changing its host traits");
    expect(view->effect().uniform("backdropLuma")->front() < 0.5f
               && std::abs(view->effect().uniform("curve")->front() - 0.23f) < 0.001f,
           "sampled appearance reaches the material and preserves optical overrides");
    draw(SK_ColorBLACK);
    SkBitmap pixels;
    pixels.allocN32Pixels(480, 320);
    surface->readPixels(pixels, 0, 0);
    expect(SkColorGetR(pixels.getColor(33, 33)) > 240,
           "dynamic foreground colors render light over a sampled black backdrop");

    auto lateContent = new_shared<UIView>();
    view->addSubview(lateContent);
    expect(lateContent->traitCollection()->userInterfaceStyle() == Style::dark,
           "content inserted after adaptation inherits the current material appearance");
    draw(SK_ColorWHITE);
    expect(ink->traitCollection()->userInterfaceStyle() == Style::light,
           "changing the live backdrop reverses the content appearance");

    draw(SK_ColorBLACK, false);
    draw(SK_ColorWHITE, false);
    DispatchQueueTestHarness::pumpMain();
    draw(SK_ColorWHITE);
    expect(ink->traitCollection()->userInterfaceStyle() == Style::light,
           "a backdrop changed during a pending transfer is sampled again");

    draw(SK_ColorBLACK);
    view->setFrame({10, 10, 300, 160});
    expect(ink->traitCollection()->userInterfaceStyle() == Style::light,
           "growing small glass to a card immediately restores inherited content traits");
    draw(SK_ColorBLACK);
    expect(view->effect().uniform("backdropLuma")->front() > 0.5f,
           "large opt-in glass keeps the app appearance over a black backdrop");

    auto animation = new_shared<CABasicAnimation>("bounds");
    animation->duration = 1;
    animation->fromValue = NXRect(0, 0, 120, 64);
    animation->toValue = NXRect(0, 0, 300, 160);
    view->layer()->add(animation, "bounds");
    view->layer()->animateAt(Timer(10));
    draw(SK_ColorBLACK);
    expect(ink->traitCollection()->userInterfaceStyle() == Style::dark,
           "small presentation bounds adapt while the animation's model bounds are already large");
    view->layer()->animateAt(Timer(2000));
    draw(SK_ColorBLACK);
    expect(ink->traitCollection()->userInterfaceStyle() == Style::light,
           "finishing the size animation restores the large material's inherited appearance");
    view->setFrame({10, 10, 120, 64});
    draw(SK_ColorBLACK, false);
    effect.setAdaptsAppearanceToBackdrop(false);
    view->setEffect(effect);
    DispatchQueueTestHarness::pumpMain();
    expect(ink->traitCollection()->userInterfaceStyle() == Style::light,
           "disabling adaptation discards a pending sample and restores inherited traits");
    expect(view->applyXMLAttribute("adaptsAppearanceToBackdrop", "true")
               && view->glassEffect()->adaptsAppearanceToBackdrop()
               && !view->applyXMLAttribute("adaptsAppearanceToBackdrop", "invalid"),
           "XML supports the opt-in flag and rejects malformed values");

    auto darkRoot = new_shared<TraitRoot>(Style::dark);
    darkRoot->addSubview(view);
    surface->getCanvas()->clear(SK_ColorWHITE);
    darkRoot->layer()->skiaRender(surface->getCanvas());
    DispatchQueueTestHarness::pumpMain();
    expect(view->traitCollection()->userInterfaceStyle() == Style::dark
               && ink->traitCollection()->userInterfaceStyle() == Style::light,
           "reparented small glass can choose light content in a dark app");
    view->setBounds({0, 0, 300, 160});
    expect(ink->traitCollection()->userInterfaceStyle() == Style::dark,
           "large glass restores the new parent's dark appearance after reparenting");

    UIGlassEffect clear(UIGlassEffect::Style::clear);
    clear.setAdaptsAppearanceToBackdrop(true);
    view->setEffect(clear);
    view->setFrame({10, 10, 120, 64});
    surface->getCanvas()->clear(SK_ColorWHITE);
    darkRoot->layer()->skiaRender(surface->getCanvas());
    DispatchQueueTestHarness::pumpMain();
    expect(ink->traitCollection()->userInterfaceStyle() == Style::dark,
           "clear glass ignores legacy appearance adaptation");
}

}
int main() {
    testBackingScale();
    testOptInGlassAppearance();
    if (failures) return 1;
    std::cout << "Glass appearance tests passed\n";
}
