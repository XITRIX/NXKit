#include <UIKitCore.h>
#include <UINib.h>
#include <UIWindow.h>
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include <iostream>
#include <stdexcept>
#include <limits>

using namespace NXKit;
namespace {
int failures = 0;
void expect(bool condition, const char* message) {
    if (!condition) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}

// A custom kit needs only Core to participate in ancestor lookup and navigation.
class CustomNavigation final : public UIViewController, public UINavigationControllerInterface {
public:
    std::vector<std::shared_ptr<UIViewController>> stack;
    std::vector<std::shared_ptr<UIViewController>> viewControllers() const override { return stack; }
    void setViewControllers(std::vector<std::shared_ptr<UIViewController>> controllers, bool) override {
        for (const auto& child : children()) child->removeFromParent();
        stack = std::move(controllers);
        for (const auto& child : stack) { addChild(child); child->didMoveToParent(shared_from_this()); }
    }
    void pushViewController(const std::shared_ptr<UIViewController>& controller, bool) override {
        addChild(controller); stack.push_back(controller); controller->didMoveToParent(shared_from_this());
    }
    std::shared_ptr<UIViewController> popViewController(bool) override {
        if (stack.empty()) return nullptr;
        auto controller = stack.back(); stack.pop_back(); controller->removeFromParent(); return controller;
    }
    std::vector<std::shared_ptr<UIViewController>> popToViewController(
        const std::shared_ptr<UIViewController>& controller, bool animated) override {
        std::vector<std::shared_ptr<UIViewController>> removed;
        while (!stack.empty() && stack.back() != controller) removed.push_back(popViewController(animated));
        return removed;
    }
    std::vector<std::shared_ptr<UIViewController>> popToRootViewController(bool animated) override {
        return stack.empty() ? std::vector<std::shared_ptr<UIViewController>>{}
                             : popToViewController(stack.front(), animated);
    }
    std::shared_ptr<UIViewController> topViewController() const override {
        return stack.empty() ? nullptr : stack.back();
    }
    std::shared_ptr<UIViewController> visibleViewController() const override { return topViewController(); }
};

void discovery() {
    auto outer = new_shared<CustomNavigation>();
    auto inner = new_shared<CustomNavigation>();
    auto wrapper = new_shared<UIViewController>();
    auto leaf = new_shared<UIViewController>();
    expect(!leaf->navigationController() && !leaf->tabBarController(), "uncontained controller has no container");
    outer->pushViewController(inner, false);
    inner->pushViewController(wrapper, false);
    wrapper->addChild(leaf);
    expect(leaf->navigationController() == inner, "nearest custom navigation is found through ordinary wrappers");
    expect(inner->navigationController() == outer, "container lookup excludes self");
    auto next = new_shared<UIViewController>();
    leaf->navigationController()->pushViewController(next, false);
    expect(inner->topViewController() == next, "navigation dispatch uses the shared capability");
    leaf->removeFromParent();
    expect(!leaf->navigationController(), "removal immediately clears lookup");
    outer->addChild(leaf);
    expect(leaf->navigationController() == outer, "reparenting immediately updates lookup");
    std::weak_ptr<CustomNavigation> weakOuter = outer;
    outer.reset();
    expect(weakOuter.expired() && !leaf->navigationController(), "lookup does not retain ancestors");
}

void neutralControl() {
    auto control = new_shared<UIControl>();
    const auto transform = NXAffineTransform::translationBy(3, 7);
    control->setTransform(transform);
    control->setAlpha(.6f);
    control->layer()->setShadowOpacity(.25f);
    control->layer()->setShadowRadius(5);
    control->layer()->setZPosition(4);
    control->willGainFocus();
    control->willChangeFocusHighlight(true);
    control->setHighlighted(true);
    expect(control->isHighlighted(), "neutral control still tracks highlighted state");
    control->setHighlighted(false);
    control->willLoseFocus();
    expect(control->transform() == transform && control->alpha() == .6f,
           "core control feedback preserves caller geometry and opacity");
    expect(control->layer()->shadowOpacity() == .25f && control->layer()->shadowRadius() == 5
           && control->layer()->zPosition() == 4, "core focus does not install a kit's shadow or elevation");
}

void tabMetadata() {
    auto screen=new_shared<UIViewController>(); screen->setTitle("Default");
    const auto item=screen->tabBarItem();
    expect(item && item->title()=="Default" && item->isEnabled() && item->tag()==0
           && !item->image() && !item->selectedImage() && !item->badgeValue() && !item->badgeColor(),
           "Core supplies style-independent tab item defaults without loading a view");
    screen->setTitle("Changed"); expect(item->title()=="Changed","default tab title follows its controller");
    item->setTitle("Explicit"); screen->setTitle("Ignored");
    expect(item->title()=="Explicit","an explicit item title takes precedence");
    item->setEnabled(false); item->setTag(9); item->setBadgeValue("2");
    expect(!item->isEnabled() && item->tag()==9 && item->badgeValue()=="2",
           "tab metadata remains independently mutable in Core");
    screen->setTabBarItem(nullptr);
    expect(screen->tabBarItem()!=item && screen->tabBarItem()->title()=="Ignored" && !screen->viewIsLoaded(),
           "resetting tab metadata recreates the default item without loading content");
}

void navigationMetadata() {
    auto screen=new_shared<UIViewController>();screen->setTitle("Root");
    const auto item=screen->navigationItem();screen->setTitle("Changed");
    expect(item->title()=="Changed"&&!screen->viewIsLoaded(),"Core navigation metadata follows titles without loading views");
    item->setTitle("Explicit");screen->setTitle("Other");
    expect(item->title()=="Explicit","explicit navigation title takes precedence");
    auto button=std::make_shared<UIBarButtonItem>(UIAction("Action"));
    item->setRightBarButtonItems({button});button->setEnabled(false);button->setWidth(80);
    expect(item->rightBarButtonItem()==button&&!button->isEnabled()&&button->width()==80,"Core bar button metadata is independently usable");
    bool invalid=false;try{item->setLeftBarButtonItems({button});}catch(const std::invalid_argument&){invalid=true;}
    expect(invalid&&item->leftBarButtonItems().empty(),"duplicate navigation buttons reject before mutation");
}

void genericBackdrop() {
    BackdropEffect effect(R"(
        uniform shader content;
        uniform float gain;
        half4 main(float2 xy) {
            half4 pixel = content.eval(xy);
            return half4(pixel.b * gain, pixel.g, pixel.r, pixel.a);
        }
    )");
    effect.setUniform("gain", 1.0f);
    auto layer = new_shared<CABackdropLayer>(effect);
    layer->setFrame({0, 0, 32, 32});
    auto copy = std::dynamic_pointer_cast<CABackdropLayer>(layer->copy());
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(32,32));
    surface->getCanvas()->clear(SK_ColorBLUE);
    layer->skiaRender(surface->getCanvas());
    SkBitmap pixels; pixels.allocN32Pixels(32,32); surface->readPixels(pixels,0,0);
    expect(pixels.getColor(16,16) == SK_ColorRED, "generic Core shader renders without UIKit");
    effect.setUniform("gain", .5f);
    layer->setEffect(effect);
    expect(copy->effect().uniform("gain")->front() == .5f, "presentation copy receives shader updates");
    bool rejected = false;
    try { layer->setEffect(BackdropEffect("invalid shader")); }
    catch (const std::invalid_argument&) { rejected = true; }
    expect(rejected && layer->effect().uniform("gain")->front() == .5f,
           "invalid shader preserves the previously installed effect");
    effect.setBackdropBlurKernel({2,5,.4f});
    effect.setBackdropInput("aux",BackdropBlurKernel{1,4,.25f});
    const auto original=effect;
    effect.setBackdropBlurRadius(3);
    effect.setBackdropInput("aux",0);
    expect(original.backdropBlurKernel()==BackdropBlurKernel{2,5,.4f}
        && original.backdropInputKernels().at("aux")==BackdropBlurKernel{1,4,.25f}
        && effect.backdropBlurKernel()==BackdropBlurKernel{3,3,0},
        "Core blur mixtures copy independently and legacy setters restore a single kernel");
    for(const auto kernel: {BackdropBlurKernel{-1,2,.5f},BackdropBlurKernel{1,-2,.5f},
        BackdropBlurKernel{1,2,-.1f},BackdropBlurKernel{1,2,1.1f},
        BackdropBlurKernel{1,2,std::numeric_limits<float>::quiet_NaN()},
        BackdropBlurKernel{std::numeric_limits<float>::infinity(),2,.5f}}) {
        bool primaryRejected=false,namedRejected=false;
        try {effect.setBackdropBlurKernel(kernel);}catch(const std::invalid_argument&){primaryRejected=true;}
        try {effect.setBackdropInput("aux",kernel);}catch(const std::invalid_argument&){namedRejected=true;}
        expect(primaryRejected&&namedRejected
            &&effect.backdropBlurKernel()==BackdropBlurKernel{3,3,0}
            &&effect.backdropInputKernels().at("aux")==BackdropBlurKernel{},
            "invalid primary and named mixtures preserve prior Core state");
    }
    effect.setBackdropPyramid(8,{{0,0,2},{1,0,2}});
    const auto pyramidCopy=effect;
    expect(effect.backdropPyramid()->taps[0][2]==.5f,"Core pyramid normalizes generic tap weights");
    for(const auto taps: {std::vector<std::array<float,3>>{},{{0,0,-1}},{{0,0,0}},{{NAN,0,1}}}) {
        bool failed=false;try{effect.setBackdropPyramid(1,taps);}catch(const std::invalid_argument&){failed=true;}
        expect(failed&&effect.backdropPyramid()->maximumRadius==8,"invalid Core pyramid preserves prior descriptor");
    }
    effect.setBackdropBlurRadius(0);
    expect(!effect.backdropPyramid()&&pyramidCopy.backdropPyramid()->taps.size()==2,
        "legacy Gaussian override clears pyramid independently of copied material");
    effect.setBackdropPyramid(8,{{0,0,1}},2,{{0,0,3},{2,0,3}});
    const auto stagedCopy=effect;
    expect(stagedCopy.backdropPyramid()->minimumRadius==2
        &&stagedCopy.backdropPyramid()->initialTaps[1][2]==.5f,
        "Core retains and normalizes an independent initial reduction stencil");
    for(float minimum:{-1.f,9.f,NAN}) {
        bool rejected=false;try{effect.setBackdropPyramid(8,{{0,0,1}},minimum);}catch(const std::invalid_argument&){rejected=true;}
        expect(rejected&&effect.backdropPyramid()->minimumRadius==2,
            "invalid pyramid ranges preserve the prior reduction plan");
    }
    effect.setBackdropBlurKernel({2,9,0});
    expect(effect.backdropBlurRadius()==2,"unused mixture branches do not inflate sampling support");
    effect.setBackdropBlurKernel({2,9,1});
    expect(effect.backdropBlurKernel()==BackdropBlurKernel{9,9,0},"full weight canonicalizes to the second kernel");
}
void genericPostShaderBlur() {
    BackdropEffect effect(R"(
        uniform shader content;
        half4 main(float2 xy) {
            half edge=xy.x<16?1:0;
            return half4(half3(edge),content.eval(xy).a);
        }
    )");
    effect.setOutputBlurRadius(2);
    auto layer=new_shared<CABackdropLayer>(effect);layer->setFrame({0,0,32,32});
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(32,32));surface->getCanvas()->clear(SK_ColorBLACK);
    layer->skiaRender(surface->getCanvas());
    SkBitmap pixels;pixels.allocN32Pixels(32,32);surface->readPixels(pixels,0,0);
    const auto left=SkColorGetR(pixels.getColor(15,16)),right=SkColorGetR(pixels.getColor(16,16));
    expect(left>right&&left<235&&right>20,
        "Core output blur filters a shader-generated edge; input blur alone cannot soften it");
    for(float radius:{-1.f,NAN,std::numeric_limits<float>::infinity()}) {
        bool rejected=false;try{effect.setOutputBlurRadius(radius);}catch(const std::invalid_argument&){rejected=true;}
        expect(rejected&&effect.outputBlurRadius()==2,"invalid output blur preserves the installed sigma");
    }
}
}
int main() {
    discovery();
    neutralControl();
    tabMetadata();
    navigationMetadata();
    genericBackdrop();
    genericPostShaderBlur();
    if (!failures) std::cout << "Core module boundary tests passed\n";
    return failures ? 1 : 0;
}
