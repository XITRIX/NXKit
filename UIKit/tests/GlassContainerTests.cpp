#include <UIGlassEffect.h>
#include <GlassContainerView.h>
#include <BackdropEffectView.h>
#include <CABasicAnimation.h>
#include "CABackdropEffectLayer.hpp"
#include "CAGlassContainerLayer.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

using namespace NXKit;
std::vector<SkRect> captureBackdropRegions(const std::function<void(SkCanvas*)>& draw);
namespace NXKit {
class GlassContainerTestHarness {
public:
    static std::shared_ptr<UITraitCollection> traits(bool dark, NXFloat scale = 2) {
        auto traits = std::make_shared<UITraitCollection>();
        traits->_userInterfaceStyle = dark ? UIUserInterfaceStyle::dark : UIUserInterfaceStyle::light;
        traits->_displayScale = scale;
        return traits;
    }
};
}
namespace {
class TraitRoot final : public UIView {
public:
    explicit TraitRoot(bool dark) : UIView(NXRect(0,0,720,760)) { _traitCollection = GlassContainerTestHarness::traits(dark); }
    void setDisplayScale(NXFloat scale) {
        auto previous = _traitCollection;
        _traitCollection = GlassContainerTestHarness::traits(previous->userInterfaceStyle() == UIUserInterfaceStyle::dark, scale);
        traitCollectionDidChange(previous);
    }
};
int failures = 0;
void expect(bool value, const std::string& message) { if (!value) { ++failures; std::cerr << "FAIL: " << message << '\n'; } }
UIGlassEffect opaque(UIColor color = UIColor::white) {
    UIGlassEffect effect;
    effect.setTintColor(color); effect.setTintOpacity(1); effect.setFrostRadius(0);
    effect.setContrast(0); effect.setLuminosity(0); effect.setRefraction(0);
    effect.setEdgeStrength(0); effect.setEdgeAbsorption(0); effect.setBrightBackdropShade(0);
    return effect;
}
std::shared_ptr<BackdropEffectView> item(NXRect frame, const UIGlassEffect& effect = opaque()) {
    auto view = new_shared<BackdropEffectView>(effect);
    view->setFrame(frame); view->layer()->setCornerRadius(std::min(frame.width(), frame.height()) * 0.5f);
    view->layer()->setShadowOpacity(0); return view;
}
SkBitmap render(const std::shared_ptr<UIView>& root, int scale = 2, int width = 320, int height = 180) {
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width * scale, height * scale));
    auto* canvas = surface->getCanvas(); canvas->clear(SK_ColorBLACK); canvas->scale(scale, scale);
    root->layer()->setContentsScale(scale);
    root->layer()->presentationOrSelf()->skiaRender(canvas);
    SkBitmap pixels; pixels.allocN32Pixels(width * scale, height * scale);
    expect(surface->readPixels(pixels, 0, 0), "container raster is readable"); return pixels;
}
int red(const SkBitmap& pixels, float x, float y, int scale = 2) { return SkColorGetR(pixels.getColor(x * scale, y * scale)); }
void save(const SkBitmap& pixels, const std::string& path) {
    std::ofstream file(path, std::ios::binary); file << "P6\n" << pixels.width() << ' ' << pixels.height() << "\n255\n";
    for (int y = 0; y < pixels.height(); ++y) for (int x = 0; x < pixels.width(); ++x) {
        const auto c = pixels.getColor(x,y); const unsigned char rgb[] = {static_cast<unsigned char>(SkColorGetR(c)), static_cast<unsigned char>(SkColorGetG(c)), static_cast<unsigned char>(SkColorGetB(c))};
        file.write(reinterpret_cast<const char*>(rgb), 3);
    }
}
void testDisconnectedFilterRegions() {
    auto container=new_shared<GlassContainerView>();container->setFrame({0,0,1000,200});
    auto a=item({20,60,60,60}),b=item({900,60,60,60});
    container->contentView()->addSubview(a);container->contentView()->addSubview(b);container->layoutIfNeeded();
    auto layer=std::static_pointer_cast<CAGlassContainerLayer>(container->layer());
    const auto capture=[&] {return captureBackdropRegions([&](SkCanvas* canvas) {layer->draw(canvas);});};
    const auto apart=capture();
    expect(apart.size()==2,"distant members filter two bounded regions instead of the empty interval");
    if(apart.size()==2)expect(apart[0].right()<apart[1].left()||apart[1].right()<apart[0].left(),"separate output passes do not overlap");
    b->setFrame({85,60,60,60});const auto joined=capture();
    expect(joined.size()==1,"near members retain a single merging/filter pass");
    b->setFrame({900,60,60,60});auto wide=opaque();wide.setMaximumSampleRadius(1000);a->setEffect(wide);b->setEffect(wide);
    const auto reachable=capture();
    expect(reachable.size()==1,"large explicit sampling reach conservatively keeps a common backdrop pass");
}
void testBehavior() {
    UIGlassContainerEffect effect;
    expect(effect.spacing() == 0, "spacing defaults to UIKit's observed zero");
    for (float invalid : {-1.0f, std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()}) {
        bool rejected = false; try { effect.setSpacing(invalid); } catch (const std::invalid_argument&) { rejected = true; }
        expect(rejected, "invalid spacing is rejected");
    }
    auto container = new_shared<GlassContainerView>(); container->setFrame({0,0,320,180});
    auto a = item({50,50,60,60}), b = item({114,50,60,60});
    container->contentView()->addSubview(a); container->contentView()->addSubview(b);
    expect(red(render(container),112,80) == 0, "default zero spacing leaves a gap");
    effect.setSpacing(20); container->setEffect(effect); effect.setSpacing(0);
    expect(container->effect().spacing() == 20, "container copies its effect");
    for (int scale : {1,2,3}) {
        const auto pixels = render(container,scale);
        expect(red(pixels,112,80,scale) > 240, "near items share a filled neck at every scale");
        expect(red(pixels,112,50,scale) == 0, "neck stays curved instead of filling the bounding box");
        int fractional = 0;
        for (int y=0; y<pixels.height(); ++y) for (int x=0; x<pixels.width(); ++x) {
            const auto value = SkColorGetR(pixels.getColor(x,y));
            if (value > 0 && value < 255) ++fractional;
        }
        expect(fractional > 20*scale, "the combined SDF edge has antialiased pixel coverage");
    }
    b->setFrame({140,50,60,60});
    expect(red(render(container),125,80) == 0, "moving items apart disconnects the material immediately");
    b->setFrame({114,50,60,60}); b->setHidden(true);
    expect(red(render(container),112,80) == 0, "hidden items leave the union"); b->setHidden(false);
    b->removeFromSuperview();
    expect(red(render(container),112,80) == 0, "removing an item removes its shape");
    auto other = new_shared<GlassContainerView>(); other->setFrame({0,0,320,180}); other->contentView()->addSubview(b);
    expect(red(render(other),144,80) > 240, "reparented glass is never left suppressed");
    container->contentView()->addSubview(b);
    auto content = new_shared<UIView>(NXRect(20,20,12,12)); content->setBackgroundColor(UIColor(0,255,0)); a->addSubview(content);
    auto pixels = render(container);
    expect(SkColorGetG(pixels.getColor(75*2,75*2)) > 240 && red(pixels,75,75) < 10, "content renders once above the combined glass");
    expect(container->hitTest({80,80},nullptr) == content, "child hit testing is preserved");
    expect(container->hitTest({112,80},nullptr) == container->contentView(), "the visual neck does not invent a child touch target");
    b->setEffect(UIGlassEffect(UIGlassEffect::Style::clear).backdropEffect());
    expect(red(render(container),112,80) == 0, "custom SkSL effects do not accidentally join UIKit glass groups");
    b->setEffect(opaque());
    auto wrapper = new_shared<UIView>(NXRect(10,0,300,180)); container->contentView()->addSubview(wrapper); wrapper->addSubview(b);
    b->setFrame({104,50,60,60});
    expect(red(render(container),112,80) > 240, "ordinary nested wrappers use container coordinates");
    wrapper->setClipsToBounds(true);
    expect(red(render(container),112,80) == 0, "masked wrapper branches retain independent glass rendering");
    wrapper->setClipsToBounds(false);
    auto animation = new_shared<CABasicAnimation>("position"); animation->duration = 1;
    animation->fromValue = NXPoint(134,80); animation->toValue = NXPoint(230,80);
    b->setFrame({200,50,60,60}); // The model is already at the separated target.
    b->layer()->add(animation,"position"); b->layer()->animateAt(Timer(1));
    expect(red(render(container),112,80) > 240, "the union follows presentation geometry rather than model frames");
    b->layer()->animateAt(Timer(2000));
    expect(red(render(container),112,80) == 0, "finishing the animation disconnects at the model target");
    b->layer()->removeAllAnimations();
    b->setFrame({104,50,60,60});
    container->setBounds({15,10,320,180});
    expect(container->contentView()->frame() == container->bounds(), "content view follows bounds including a nonzero origin");
    container->setBounds({0,0,320,180});
    expect(container->applyXMLAttribute("spacing","30") && container->effect().spacing() == 30
        && !container->applyXMLAttribute("spacing","-1"), "XML spacing validates and installs the effect");
    // Nested containers never merge with an outer group.
    auto nested = new_shared<GlassContainerView>(); nested->setFrame({0,0,320,180});
    wrapper->removeFromSuperview(); container->contentView()->addSubview(nested); nested->contentView()->addSubview(b); b->setFrame({114,50,60,60});
    expect(red(render(container),112,80) == 0, "nested container boundaries isolate groups");
    // Affine transforms and bounds origins use the same matrix as content.
    nested->removeFromSuperview(); container->contentView()->addSubview(b);
    b->setFrame({140,50,60,60}); b->setTransform(NXAffineTransform::translationBy(-26,0));
    expect(red(render(container),112,80) > 240, "affine item transforms participate in the field");
    b->setTransform(NXAffineTransform::identity);
    Internal::GlassInteraction press; press.inflation.value = 1;
    std::static_pointer_cast<CABackdropEffectLayer>(a->layer())->setGlassInteraction(press);
    expect(red(render(container),114,80) > 240, "the union includes interactive inflation without changing hit geometry");
    std::static_pointer_cast<CABackdropEffectLayer>(a->layer())->setGlassInteraction({});
    b->setFrame({114,60,100,40}); b->layer()->setCornerRadius(20);
    expect(red(render(container),112,80) > 240, "a nearby capsule joins the group");
    b->setTransform(NXAffineTransform::rotationBy(90));
    expect(red(render(container),112,80) == 0, "rotating an item recomputes the field from its true shape");
    b->setTransform(NXAffineTransform::identity);
    b->setFrame({100,50,60,60}); b->layer()->setCornerRadius(30);
    auto veil = opaque(); veil.setTintOpacity(.4f); a->setEffect(veil); b->setEffect(veil);
    a->subviews().front()->removeFromSuperview();
    pixels = render(container);
    expect(std::abs(red(pixels,80,80)-red(pixels,105,80)) <= 2,
           "overlaps sample one backdrop without doubling the tint or blur");
    a->setEffect(opaque(UIColor(255,0,0))); b->setEffect(opaque(UIColor(0,0,255)));
    pixels = render(container);
    const auto join = pixels.getColor(105*2,80*2);
    expect(SkColorGetR(join)<5 && SkColorGetB(join)>180, "the front tint owns the interior of an overlap");
    const auto seam=pixels.getColor(94*2,80*2);
    expect(SkColorGetR(seam)>20 && SkColorGetB(seam)>20, "neighbor tints blend only near the front boundary");
    a->layer()->setZPosition(1);
    const auto reversed=render(container).getColor(105*2,80*2);
    expect(SkColorGetR(reversed)>180 && SkColorGetB(reversed)<5, "reordering updates tint ownership");
    a->layer()->setZPosition(0);
    a->setEffect(opaque()); b->setEffect(opaque());
    a->setAlpha(.5f); b->setAlpha(.5f);
    pixels = render(container);
    expect(std::abs(red(pixels,80,80)-red(pixels,105,80)) <= 2 && red(pixels,80,80) > 100
               && red(pixels,80,80)<150, "item opacity applies once to the merged material");
    a->setAlpha(1); b->setAlpha(1);
    auto third = item({164,50,60,60}); container->contentView()->addSubview(third);
    expect(red(render(container),162,80) > 240, "three items can form a connected group");
    container->setClipsToBounds(true); container->setFrame({0,0,150,180});
    expect(red(render(container),160,80) == 0, "container clipping clips both the field and its children");
    third->setFrame({164,50,0,0});
    render(container);
    expect(std::dynamic_pointer_cast<CAGlassContainerLayer>(container->layer()->copy()) != nullptr,
           "presentation copies preserve the container layer type");

}
void testBackingScale() {
    auto root = new_shared<TraitRoot>(false);
    auto container = new_shared<GlassContainerView>();
    root->addSubview(container);
    expect(container->layer()->contentsScale() == 2, "container inherits Retina screen backing resolution");
    auto animation = std::make_shared<CABasicAnimation>("bounds");
    animation->duration = 1; animation->fromValue = NXRect(0,0,52,52); animation->toValue = NXRect(0,0,80,80);
    container->layer()->add(animation,"bounds");
    root->setDisplayScale(3);
    expect(container->layer()->contentsScale() == 3 && container->layer()->presentationOrSelf()->contentsScale() == 3,
        "container screen change updates model and active presentation filter resolution");
    container->layer()->setContentsScale(1.5f);
    root->setDisplayScale(3); container->setFrame({0,0,100,100}); container->layoutSubviews();
    expect(container->layer()->contentsScale() == 1.5f, "same-screen notifications and layout preserve an explicit container scale");
    root->setDisplayScale(0);
    expect(container->layer()->contentsScale() == 1, "invalid display scale resolves to one for containers");
    auto other = new_shared<TraitRoot>(true); other->addSubview(container);
    expect(container->layer()->contentsScale() == 2, "reparented container adopts the new display resolution");
}
// Material blending is verified separately from native optics: a flat
// backdrop makes every exact material spatially constant and exposes switches
// caused by interpolating enum-like shader flags instead of finished colors.
void testExactMaterialHandoff() {
    for (bool dark : {false,true}) for (bool reverse : {false,true}) {
        auto root = new_shared<TraitRoot>(dark); root->setBackgroundColor(UIColor(96,96,96));
        UIGlassContainerEffect grouping; grouping.setSpacing(20);
        auto container = new_shared<GlassContainerView>(grouping); container->setFrame({0,0,320,180}); root->addSubview(container);
        UIGlassEffect neutral, blue; blue.setTintColor(UIColor(0,122,255));
        for (auto* material : {&neutral,&blue}) {
            material->setFrostRadius(0); material->setRefraction(0);
            material->setEdgeStrength(0); material->setEdgeAbsorption(0); material->setBrightBackdropShade(0);
        }
        auto back = item({50,50,60,60}, neutral);
        auto front = item({100,50,60,60}, blue);
        if (reverse) { container->contentView()->addSubview(front); container->contentView()->addSubview(back); }
        else { container->contentView()->addSubview(back); container->contentView()->addSubview(front); }
        const auto initial = render(root,3);
        const auto backColor = initial.getColor(70*3,80*3), frontColor = initial.getColor(140*3,80*3);
        const auto channel = [](SkColor color,int shift) { return int((color >> shift)&255); };
        for (float ramp : {.249f,.251f,.499f,.501f,.749f,.751f}) {
            // At this point the back shape owns all geometry weight and its
            // interior. Check exact colors on either side of categorical
            // thresholds, while the measured ownership handoff stays smooth.
            const float distance = 9.5f-8*ramp;
            const float frontWeight = ramp*ramp*(3-2*ramp);
            front->setFrame({94+.5f/3+distance,50,60,60});
            const auto color = render(root,3).getColor(94*3,80*3);
            for (int shift : {0,8,16}) {
                const float expected = channel(backColor,shift)*(1-frontWeight) + channel(frontColor,shift)*frontWeight;
                expect(std::abs(channel(color,shift)-expected)<=3,
                    "explicit regular tint handoff blends exact colors continuously in either insertion order/theme");
            }
        }
    }
}
// Native retained-pair fixtures show that the bridge carries unlit tint
// outside the pressed item's own silhouette; its interaction highlight does
// not spread into that extruded material. Both creation orders are measured.
void testJoinLightingCoverage() {
    for (bool dark : {false,true}) for (bool reverse : {false,true}) {
        auto root = new_shared<TraitRoot>(dark); root->setBackgroundColor(UIColor(128,128,128));
        UIGlassContainerEffect grouping; grouping.setSpacing(20);
        auto container = new_shared<GlassContainerView>(grouping); container->setFrame({0,0,320,180}); root->addSubview(container);
        UIGlassEffect neutral, blue; blue.setTintColor(UIColor(0,122,255));
        for (auto* material : {&neutral,&blue}) {
            material->setFrostRadius(0); material->setRefraction(0);
            material->setEdgeStrength(0); material->setEdgeAbsorption(0); material->setBrightBackdropShade(0);
        }
        auto a = item({50,50,60,60},neutral), b = item({112,50,60,60},blue);
        if (reverse) { container->contentView()->addSubview(b); container->contentView()->addSubview(a); }
        else { container->contentView()->addSubview(a); container->contentView()->addSubview(b); }
        const auto settled = render(root,3);
        Internal::GlassInteraction press; press.glowProgress=1; press.location={30,30};
        auto glassLayer=std::static_pointer_cast<CABackdropEffectLayer>(b->layer()); glassLayer->setGlassInteraction(press);
        const auto held = render(root,3);
        for (int x : {104,106,108}) expect(settled.getColor(x*3,80*3)==held.getColor(x*3,80*3),
            "joined press highlight stays inside its own item silhouette, leaving the tinted bridge unlit");
        expect(SkColorGetR(held.getColor(132*3,80*3))>SkColorGetR(settled.getColor(132*3,80*3))+30,
            "press highlight still illuminates the tinted item body");
        glassLayer->setGlassInteraction({});
        const auto released = render(root,3);
        expect(released.getColor(106*3,80*3)==settled.getColor(106*3,80*3),
            "items that remain merged after release recover the original material handoff");
    }
}
void testSharedJoinLighting() {
    for (bool dark : {false,true}) {
        auto root = new_shared<TraitRoot>(dark); root->setBackgroundColor(UIColor(128,128,128));
        UIGlassContainerEffect grouping; grouping.setSpacing(8);
        auto container = new_shared<GlassContainerView>(grouping); container->setFrame({0,0,320,180}); root->addSubview(container);
        UIGlassEffect blue, coral; blue.setTintColor(UIColor(0,122,255)); coral.setTintColor(UIColor(255,59,48));
        for (auto* material : {&blue,&coral}) {
            material->setFrostRadius(0); material->setRefraction(0);
            material->setEdgeStrength(0); material->setEdgeAbsorption(0); material->setBrightBackdropShade(0);
        }
        auto front = item({50,50,60,60},blue), rear = item({100,50,60,60},coral);
        container->contentView()->addSubview(rear); container->contentView()->addSubview(front);
        const auto settled = render(root,3);
        Internal::GlassInteraction press; press.glowProgress=1; press.location={5,30};
        auto source = std::static_pointer_cast<CABackdropEffectLayer>(rear->layer()); source->setGlassInteraction(press);
        const auto held = render(root,3);
        const auto before=settled.getColor(104*3,80*3), after=held.getColor(104*3,80*3);
        expect(SkColorGetB(before)>SkColorGetR(before)+80, "front blue owns the overlap before its coral neighbor is pressed");
        expect(SkColorGetR(after)>SkColorGetR(before)+30,
            "pressing the rear coral illuminates the front blue material within the pressed silhouette");
        expect(settled.getColor(80*3,80*3)==held.getColor(80*3,80*3),
            "shared feedback never illuminates a neighbor outside the pressed source silhouette");
        source->setGlassInteraction({});
        expect(render(root,3).getColor(104*3,80*3)==before,
            "shared highlight disappears on release while the two tinted items remain joined");
    }
}
void testPressMaterialResponse() {
    for (bool dark : {false,true}) for (bool tinted : {false,true}) for (bool clear : {false,true}) {
        auto root=new_shared<TraitRoot>(dark); root->setBackgroundColor(UIColor(128,128,128));
        UIGlassEffect glass(clear ? UIGlassEffect::Style::clear : UIGlassEffect::Style::regular); if (tinted) glass.setTintColor(UIColor(0,122,255));
        glass.setEdgeStrength(0); glass.setEdgeAbsorption(0); glass.setBrightBackdropShade(0);
        glass.setRefraction(0); glass.setFrostRadius(0);
        auto view=item({40,40,52,52},glass); root->addSubview(view);
        const auto settled=render(root,2);
        Internal::GlassInteraction lighting; lighting.glowProgress=1; lighting.location={26,26};
        std::static_pointer_cast<CABackdropEffectLayer>(view->layer())->setGlassInteraction(lighting);
        const auto held=render(root,2);
        double numerator=0,denominator=0;
        for (int y : {56,58,74,76}) for (int x : {56,58,74,76}) {
            const auto a=settled.getColor(x*2,y*2),b=held.getColor(x*2,y*2);
            for (int shift : {0,8,16}) {
                const int base=(a>>shift)&255,down=(b>>shift)&255;
                const int headroom=255-base;
                if (headroom>20) { numerator+=(down-base)*headroom; denominator+=headroom*headroom; }
            }
        }
        const double opacity=numerator/denominator;
        const double expected=clear ? (dark ? .212 : .278)
            : (tinted ? (dark ? .349 : .364) : (dark ? .439 : .502));
        expect(std::abs(opacity-expected)<.012,clear
            ? "regular press calibration preserves the existing clear press response"
            : "regular neutral/tint press overlays match native held-minus-settled opacity in both themes");
    }
}
void testSingleItemEquivalence() {
    for (bool dark : {false,true}) for (int scale : {1,2,3}) for (bool pressed : {false,true}) {
        auto root = new_shared<TraitRoot>(dark); root->setBackgroundColor(UIColor(96,96,96));
        UIGlassEffect glass; glass.setTintColor(UIColor(0,122,255));
        auto view = item({80,50,60,60},glass);
        view->setTransform(NXAffineTransform(1.1f,0,0,.9f,0,0));
        if (pressed) {
            Internal::GlassInteraction motion; motion.pressed=true; motion.inflation.value=.75;
            motion.dragX.value=12; motion.dragY.value=-4; motion.stretchX.value=12; motion.stretchY.value=4; motion.location={35,25}; motion.glowProgress=.7;
            std::static_pointer_cast<CABackdropEffectLayer>(view->layer())->setGlassInteraction(motion);
        }
        root->addSubview(view);
        const auto independent = render(root,scale);
        auto group = new_shared<GlassContainerView>(); group->setFrame({0,0,320,180}); root->addSubview(group);
        group->contentView()->addSubview(view);
        const auto grouped = render(root,scale);
        int maximumDifference=0;
        for (int y=0;y<grouped.height();++y) for (int x=0;x<grouped.width();++x) {
            const auto a=independent.getColor(x,y), b=grouped.getColor(x,y);
            for (int shift : {0,8,16,24}) maximumDifference=std::max(maximumDifference,
                std::abs(int((a>>shift)&255)-int((b>>shift)&255)));
        }
        expect(maximumDifference==0,"one-item container preserves standalone optical/AA output under affine and press transforms");
    }
}
void testNeckDistanceCoverage() {
    auto group = new_shared<GlassContainerView>(); group->setFrame({0,0,320,180});
    UIGlassContainerEffect effect; effect.setSpacing(8); group->setEffect(effect);
    auto a=item({50,54,52,52}), b=item({114,54,52,52});
    a->layer()->setCornerCurve(CALayerCornerCurve::circular);
    b->layer()->setCornerCurve(CALayerCornerCurve::circular);
    b->setTransform(NXAffineTransform(69.0f/52,0,0,69.0f/52,0,0));
    group->contentView()->addSubview(a); group->contentView()->addSubview(b);
    const auto pixels=render(group,3);
    const auto field=[](double x,double y) {
        const double ax=x-76, ay=y-80, bx=x-140, by=y-80;
        const double al=std::hypot(ax,ay), bl=std::hypot(bx,by);
        const double da=al-26, db=bl-34.5;
        const double k=8*std::clamp(.5-.5*(ax*bx+ay*by)/(al*bl),0.0,1.0);
        const double h=k>1e-4 ? std::clamp(.5+.5*(db-da)/k,0.0,1.0) : (da<=db ? 1.0 : 0.0);
        return db*(1-h)+da*h-k*h*(1-h);
    };
    int checked=0;
    for (int y=240;y<282;++y) for (int x=306;x<327;++x) {
        const double px=(x+.5)/3,py=(y+.5)/3,epsilon=.25/3;
        const double dx=(field(px+epsilon,py)-field(px-epsilon,py))/(2*epsilon);
        const double dy=(field(px,py+epsilon)-field(px,py-epsilon))/(2*epsilon);
        const double coverage=std::clamp(.5-field(px,py)/std::max(std::hypot(dx,dy),.05)*3,0.0,1.0);
        if (coverage>.1 && coverage<.9) {
            ++checked;
            expect(std::abs(SkColorGetR(pixels.getColor(x,y))-coverage*255)<=4,
                "neck AA uses the field gradient's physical distance without changing the zero contour");
        }
    }
    expect(checked>=3,"neck regression samples several fractional-coverage pixels");
}
void testPressedMaterialScale() {
    // A distant second member must not change an item's exact material or
    // inflation transform merely because it now takes the group shader path.
    for (bool dark : {false,true}) {
        auto root = new_shared<TraitRoot>(dark); root->setBackgroundColor(UIColor(96,96,96));
        UIGlassContainerEffect grouping; grouping.setSpacing(8);
        auto group = new_shared<GlassContainerView>(grouping); group->setFrame({0,0,320,180}); root->addSubview(group);
        UIGlassEffect blue; blue.setTintColor(UIColor(0,122,255)); blue.setEdgeStrength(0); blue.setEdgeAbsorption(0); blue.setRefraction(0);
        auto a=item({40,50,52,52},blue); group->contentView()->addSubview(a);
        Internal::GlassInteraction press; press.inflation.value=1; press.dragX.value=12; press.dragY.value=-4; press.stretchX.value=12; press.stretchY.value=4;
        press.location={40,12}; press.glowProgress=.75;
        std::static_pointer_cast<CABackdropEffectLayer>(a->layer())->setGlassInteraction(press);
        const auto standalone=render(root,3);
        group->contentView()->addSubview(item({220,50,52,52},blue));
        const auto grouped=render(root,3);
        // Compare safely inside the silhouette: the shader and path renderer
        // intentionally use different coverage implementations at the rim.
        const auto transform=press.transform(a->bounds().size);
        for (int y : {16,20,28,34}) for (int x : {16,20,28,34}) {
            const auto point=NXPoint(x,y).applying(transform)+NXPoint(40,50);
            const int pixelX=point.x*3,pixelY=point.y*3;
            const auto ca=standalone.getColor(pixelX,pixelY), cb=grouped.getColor(pixelX,pixelY);
            for (int shift : {0,8,16}) expect(std::abs(int((ca>>shift)&255)-int((cb>>shift)&255))<=2,
                "pressed/scaled material retains local optical coordinates with another group member");
        }
    }
}
void testSharedNeutralMaterialSizing() {
    for (bool dark : {false,true}) for (auto backdrop : {UIColor::white, UIColor(128,128,128)}) {
        auto root = new_shared<TraitRoot>(dark); root->setBackgroundColor(backdrop);
        auto group = new_shared<GlassContainerView>(); group->setFrame({0,0,320,200}); root->addSubview(group);
        UIGlassEffect neutral; neutral.setFrostRadius(0);
        auto small = item({24,40,52,52}, neutral); group->contentView()->addSubview(small);
        const auto ordinary = render(root);
        auto large = item({130,10,160,160}, neutral); group->contentView()->addSubview(large);
        const auto shared = render(root);
        // Native mixed-size neutral groups share their largest member's body
        // response even when disconnected. Each lens still uses its own frame.
        expect(std::abs(red(shared,50,66)-red(shared,210,90)) <= 1,
            "neutral regular members share the largest member's material defaults");
        large->setFrame({130,10,52,52});
        expect(red(render(root),50,66) == red(ordinary,50,66),
            "resizing the largest member updates shared material defaults immediately");
        large->setFrame({130,10,160,160});
        large->setHidden(true);
        expect(red(render(root),50,66) == red(ordinary,50,66),
            "hiding the largest member restores the smaller material immediately");
        large->setHidden(false);
        for (int kind : {0,1,2}) {
            UIGlassEffect effect(kind==1 ? UIGlassEffect::Style::clear : UIGlassEffect::Style::regular);
            effect.setFrostRadius(0);
            if (kind==0) effect.setTintColor(UIColor(0,122,255));
            if (kind==2) {
                effect.setFrostRadius(0); effect.setRefraction(0); effect.setRefractionDepth(.7f);
                effect.setSaturation(1); effect.setContrast(1); effect.setLuminosity(.1f);
                effect.setTintOpacity(.25f); effect.setBrightBackdropShade(0);
                effect.setEdgeStrength(0); effect.setEdgeAbsorption(0);
            }
            small->setEffect(effect); large->setHidden(true); const auto alone = render(root);
            large->setHidden(false); const auto together = render(root);
            expect(alone.getColor(100,132) == together.getColor(100,132),
                "shared neutral sizing preserves explicit tints, clear style and optical overrides");
        }
    }
}
void reference(const std::string& output, float offset, float factor, bool dark) {
    auto root = new_shared<TraitRoot>(dark); root->setBackgroundColor(UIColor(31,31,31));
    const float spacings[] = {0,10,20,40}, gaps[] = {4,12,24,44};
    for (int row=0; row<4; ++row) for (int col=0; col<4; ++col) {
        UIGlassContainerEffect effect; effect.setSpacing(spacings[col]);
        auto container = new_shared<GlassContainerView>(effect); container->setFrame({col*180.0f,row*150.0f,180,150});
        container->layer()->setContentsScale(2); root->addSubview(container);
        const float size=60*factor, gap=gaps[row]+offset, x=(180-2*size-gap)*0.5f;
        UIGlassEffect glass; glass.setTintColor(UIColor::systemRed);
        for (int i=0;i<2;++i) container->contentView()->addSubview(item({x+i*(size+gap),75-size*0.5f,size,size},glass));
    }
    for (int col=0;col<2;++col) {
        UIGlassContainerEffect effect; effect.setSpacing(20);
        auto container = new_shared<GlassContainerView>(effect); container->setFrame({col*360.0f,610,360,140});
        container->layer()->setContentsScale(2); root->addSubview(container);
        UIGlassEffect glass; glass.setTintColor(UIColor::systemRed);
        for (int i=0;i<2;++i) { auto view=item({30+i*(col?170.0f:80.0f),20,120,100},glass); view->layer()->setCornerRadius(35); container->contentView()->addSubview(view); }
    }
    save(render(root,2,720,760),output);
}
}
int main(int argc,char** argv) {
    testDisconnectedFilterRegions();
    if (argc < 3) { testBackingScale(); testBehavior(); testExactMaterialHandoff(); testJoinLightingCoverage(); testSharedJoinLighting(); testPressMaterialResponse(); testSingleItemEquivalence(); testNeckDistanceCoverage(); testPressedMaterialScale(); testSharedNeutralMaterialSizing(); }
    if (argc>1) reference(argv[1],argc>2?std::stof(argv[2]):0,argc>3?std::stof(argv[3]):1,argc>4?std::stoi(argv[4])!=0:false);
    if (failures) return 1;
    std::cout << "Glass container tests passed\n";
}
