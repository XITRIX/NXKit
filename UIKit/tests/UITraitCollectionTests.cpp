#include <UITraitCollection.h>
#include <UIUserInterfaceSizeClass.h>
#include <UIWindow.h>
#include <UIViewController.h>
#include <UIApplication.h>
#include <SkiaCtx.h>
#include <BackdropEffectView.h>
#include "CABackdropEffectLayer.hpp"
#include <DispatchQueue.h>
#include <SDL3/SDL.h>
#include "include/core/SkSurface.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>

using namespace NXKit;
using Class = UIUserInterfaceSizeClass;
using Classes = std::pair<Class,Class>;
namespace NXKit {
bool applicationRunLoop();
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>) { return true; }
class SizeClassTestHarness {
public:
    static void refresh(const std::shared_ptr<UIWindow>& window) {
        window->setFrame({NXPoint::zero,SkiaCtx::main()->getSize()});
        window->updateTraitsFromPlatform();
    }
};
class DispatchQueueTestHarness { public: static void pump() { DispatchQueue::main()->performAll(); } };
}
namespace {
int failures=0;
void expect(bool result,const char* message) { if (!result) { ++failures;std::cerr<<"FAIL: "<<message<<'\n'; } }
Classes classes(const std::shared_ptr<UITraitCollection>& traits) {
    return traits ? Classes{traits->horizontalSizeClass(),traits->verticalSizeClass()}
                  : Classes{Class::unspecified,Class::unspecified};
}
class Context final:public SkiaCtx {
public:
    NXSize size{390,844};float scale=2;UIUserInterfaceStyle style=UIUserInterfaceStyle::light;
    NXFloat glassTintAmount=UITraitCollection::defaultGlassTintAmount;
    std::optional<Classes> nativeClasses;
    NXSize getSize() override { return size; }
    float getScaleFactor() override { return scale; }
    UIUserInterfaceStyle getThemeMode() override { return style; }
    NXFloat getGlassTintAmount() override { return glassTintAmount; }
    Classes getSizeClasses() override { return nativeClasses.value_or(SkiaCtx::getSizeClasses()); }
    sk_sp<SkSurface> getBackbufferSurface() override { return nullptr; }
    void swapBuffers() override {}
    void sleepForNextFrame() override {}
    bool platformRunLoop(std::function<bool()>) override { return false; }
protected:
    skgpu::graphite::Context* graphiteContext() override { return nullptr; }
    skgpu::graphite::Recorder* graphiteRecorder() override { return nullptr; }
};
class ProbeView:public UIView {
public:
    int notifications=0,layouts=0;
    std::shared_ptr<UITraitCollection> previous;
    Classes duringLayout, duringCallback;
    std::function<void()> onChange;
    void traitCollectionDidChange(std::shared_ptr<UITraitCollection> old) override {
        ++notifications;previous=old;duringCallback=classes(UITraitCollection::current());
        UIView::traitCollectionDidChange(old);
        if (onChange) onChange();
    }
    void layoutSubviews() override {
        UIView::layoutSubviews();++layouts;duringLayout=classes(traitCollection());
    }
};
class ProbeController:public UIViewController {
public:
    std::shared_ptr<ProbeView> root,leaf;
    int notifications=0;Classes atLoad,leafAtLoad;
    std::shared_ptr<UITraitCollection> previous;
    void loadView() override {
        root=new_shared<ProbeView>();leaf=new_shared<ProbeView>();root->addSubview(leaf);setView(root);
    }
    void viewDidLoad() override { atLoad=classes(traitCollection());leafAtLoad=classes(leaf->traitCollection()); }
    void traitCollectionDidChange(std::shared_ptr<UITraitCollection> old) override {
        ++notifications;previous=old;UIViewController::traitCollectionDidChange(old);
    }
};
class InheritedRoot:public UIView {
public: explicit InheritedRoot(const std::shared_ptr<UITraitCollection>& traits) { _traitCollection=traits; }
};
void testPolicy(Context& ctx) {
    struct Sample { NXSize size;Classes expected; };
    const Sample samples[]={{{390,844},{Class::compact,Class::regular}},{{844,390},{Class::regular,Class::compact}},
        {{1280,720},{Class::regular,Class::regular}},{{599,479},{Class::compact,Class::compact}},
        {{600,480},{Class::regular,Class::regular}},{{0,480},{Class::unspecified,Class::regular}},
        {{600,0},{Class::regular,Class::unspecified}},{{-1,480},{Class::unspecified,Class::regular}},
        {{std::numeric_limits<float>::infinity(),480},{Class::unspecified,Class::regular}},
        {{600,std::numeric_limits<float>::quiet_NaN()},{Class::regular,Class::unspecified}}};
    for (const auto& sample:samples) {
        ctx.size=sample.size;expect(ctx.getSizeClasses()==sample.expected,"portable classes use independent valid logical axes");
    }
    ctx.size={390,844};ctx.scale=3;ctx.setExtraScaleFactor(1.3f);
    expect(ctx.getSizeClasses()==Classes{Class::compact,Class::regular},"pixel density does not change logical size classes");
    ctx.scale=2;ctx.setExtraScaleFactor(1);
}
void testInheritance(const std::shared_ptr<Context>& ctx) {
    UITraitCollection empty;
    expect(classes(empty.copy())==Classes{Class::unspecified,Class::unspecified},"standalone collections start unspecified and copy both axes");
    auto window=new_shared<UIWindow>();auto controller=new_shared<ProbeController>();auto child=new_shared<ProbeController>();
    controller->addChild(child);window->setRootViewController(controller);
    auto chrome=new_shared<ProbeView>();window->addSubview(chrome);
    window->makeKeyAndVisible();controller->root->addSubview(child->view());
    const Classes portrait{Class::compact,Class::regular};
    expect(controller->atLoad==portrait && controller->leafAtLoad==portrait,"root and ordinary descendants have size classes in viewDidLoad");
    expect(child->atLoad==portrait && classes(child->traitCollection())==portrait,"child controllers inherit before their view loads");
    expect(classes(chrome->traitCollection())==portrait,"window-owned views receive initial traits");
    auto mask=new_shared<ProbeView>();controller->leaf->setMask(mask);
    expect(classes(mask->traitCollection())==portrait,"new masks inherit traits");
    auto initial=window->traitCollection();auto copy=initial->copy();
    expect(classes(copy)==portrait && copy!=initial && copy->displayScale()==2,"copies preserve axes and display traits independently");
    ctx->size={600,900};SizeClassTestHarness::refresh(window);
    const Classes wide{Class::regular,Class::regular};
    expect(classes(window->traitCollection())==wide && classes(controller->leaf->traitCollection())==wide
        && classes(child->leaf->traitCollection())==wide && classes(mask->traitCollection())==wide,"resizing propagates both axes through controllers, views and masks");
    expect(classes(controller->previous)==portrait && classes(initial)==portrait && classes(copy)==portrait,"previous and copied collections remain stable snapshots");
    expect(controller->root->duringCallback==wide,"trait callbacks run in the new current environment");
    auto extra=new_shared<ProbeView>();controller->root->addSubview(extra);
    expect(classes(extra->traitCollection())==wide,"views added after resize inherit the current classes");
    window->drawAndLayoutTreeIfNeeded();
    expect(controller->leaf->duringLayout==wide && child->leaf->duringLayout==wide,"layout observes the new classes after delivery");
    controller->leaf->setFrame({0,0,20,20});
    expect(classes(controller->leaf->traitCollection())==wide,"small view bounds do not reclassify inherited window traits");
    auto stable=window->traitCollection();const int before=controller->notifications;
    ctx->size={650,950};SizeClassTestHarness::refresh(window);
    expect(window->traitCollection()==stable && controller->notifications==before,"resizing within the same classes does not emit duplicate trait notifications");
    ctx->size={950,390};SizeClassTestHarness::refresh(window);
    expect(classes(child->leaf->traitCollection())==Classes{Class::regular,Class::compact},"rotation updates vertical compactness");
    ctx->size={390,950};SizeClassTestHarness::refresh(window);
    expect(classes(controller->traitCollection())==portrait,"rotating back restores portrait classes");
    ctx->nativeClasses=Classes{Class::regular,Class::compact};SizeClassTestHarness::refresh(window);
    expect(classes(controller->traitCollection())==*ctx->nativeClasses,"backend traits take precedence over geometric guesses");
    ctx->nativeClasses=wide;const auto previous=window->traitCollection();SizeClassTestHarness::refresh(window);
    expect(classes(controller->traitCollection())==wide && previous!=window->traitCollection(),"native classes can change without changing window dimensions");
    ctx->scale=3;ctx->style=UIUserInterfaceStyle::dark;SizeClassTestHarness::refresh(window);
    expect(classes(controller->traitCollection())==wide && controller->traitCollection()->displayScale()==3
        && controller->traitCollection()->userInterfaceStyle()==UIUserInterfaceStyle::dark,"density/theme updates preserve size classes");
    auto replacement=new_shared<ProbeController>();window->setRootViewController(replacement);window->makeKeyAndVisible();
    expect(replacement->atLoad==wide && replacement->leafAtLoad==wide,"replacement roots inherit existing window traits before loading");
    // Change hierarchy from a callback. Removed nodes must not receive a stale
    // delivery from a traversal that started under their former parent.
    auto mutator=new_shared<ProbeView>(),removed=new_shared<ProbeView>();replacement->root->addSubview(mutator);replacement->root->addSubview(removed);
    const int removedBefore=removed->notifications;
    mutator->onChange=[removed] { removed->removeFromSuperview(); };
    ctx->nativeClasses=portrait;SizeClassTestHarness::refresh(window);
    expect(removed->notifications==removedBefore,"trait callback removal does not invalidate traversal or deliver stale traits");
    auto other=new_shared<InheritedRoot>(previous);other->addSubview(extra);
    expect(classes(extra->traitCollection())==classes(previous),"reparenting adopts the new parent's environment");
    auto late=new_shared<ProbeController>();replacement->addChild(late);late->loadViewIfNeeded();
    expect(late->atLoad==portrait,"late child controllers inherit current traits");
    auto presented=new_shared<ProbeController>();replacement->present(presented,false);
    ctx->nativeClasses=wide;SizeClassTestHarness::refresh(window);
    expect(classes(presented->traitCollection())==wide && classes(presented->leaf->traitCollection())==wide,"active presentations inherit resize updates");
    presented->dismiss(false);
    window->setRootViewController(nullptr);
    ctx->nativeClasses.reset();ctx->size={390,844};ctx->style=UIUserInterfaceStyle::light;ctx->scale=2;
}
void testRunLoop(const std::shared_ptr<Context>& ctx) {
    ctx->size={390,844};ctx->nativeClasses.reset();
    auto window=new_shared<UIWindow>();auto controller=new_shared<ProbeController>();
    window->setRootViewController(controller);window->makeKeyAndVisible();
    ctx->size={844,390};
    SDL_Event resize{};resize.type=SDL_EVENT_WINDOW_RESIZED;
    expect(SDL_PushEvent(&resize),"a resize event reaches the application queue");
    expect(applicationRunLoop(),"the actual application frame processes a rotation/resize");
    expect(classes(controller->traitCollection())==Classes{Class::regular,Class::compact}
        && controller->leaf->duringLayout==Classes{Class::regular,Class::compact},"the frame installs rotation traits before controller/view layout");
    ctx->nativeClasses=Classes{Class::compact,Class::regular};
    expect(applicationRunLoop(),"the next application frame completes without a geometry event");
    expect(classes(controller->traitCollection())==*ctx->nativeClasses,"same-bounds native trait changes are polled without relying on SDL resize events");
    const int before=controller->notifications;
    expect(applicationRunLoop() && controller->notifications==before,"unchanged application frames do not repeat trait delivery");
    window->setRootViewController(nullptr);ctx->nativeClasses.reset();ctx->size={390,844};
}

void testGlass(const std::shared_ptr<Context>& ctx) {
    ctx->glassTintAmount=.25f;
    auto window=new_shared<UIWindow>();auto controller=new_shared<ProbeController>();window->setRootViewController(controller);window->makeKeyAndVisible();
    UIGlassEffect effect;effect.setAdaptsAppearanceToBackdrop(true);auto glass=new_shared<BackdropEffectView>(effect);glass->setFrame({0,0,120,64});
    auto ink=new_shared<ProbeView>();glass->addSubview(ink);controller->root->addSubview(glass);
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(200,100));
    const auto sample=[&] { surface->getCanvas()->clear(SK_ColorBLACK);glass->layer()->skiaRender(surface->getCanvas());DispatchQueueTestHarness::pump(); };
    sample();sample();expect(ink->traitCollection()->userInterfaceStyle()==UIUserInterfaceStyle::dark,"small opt-in glass establishes a content appearance override");
    ctx->size={900,700};SizeClassTestHarness::refresh(window);sample();sample();
    expect(classes(ink->traitCollection())==Classes{Class::regular,Class::regular}
        && ink->traitCollection()->userInterfaceStyle()==UIUserInterfaceStyle::dark
        && ink->traitCollection()->glassTintAmount()==.25f,"glass appearance copies retain updated size classes and the inherited glass preference");
    window->setRootViewController(nullptr);ctx->glassTintAmount=.5f;
}

void testGlassPreference(const std::shared_ptr<Context>& ctx) {
    const auto app=UIApplication::shared;
    expect(UITraitCollection().glassTintAmount()==.5f && !app->overrideGlassTintAmount(),
           "the portable default preserves the current glass material without an app override");
    for(const auto amount : {0.f,.25f,.5f,.75f,1.f}) {
        auto traits=UITraitCollection::traitCollectionWithGlassTintAmount(amount);
        expect(traits->glassTintAmount()==amount && traits->copy()->glassTintAmount()==amount,
               "standalone preference traits and their copies preserve the full slider range");
    }
    for(const auto invalid : {-1.f,1.01f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}) {
        bool traitRejected=false,appRejected=false;
        try { (void)UITraitCollection::traitCollectionWithGlassTintAmount(invalid); } catch(const std::invalid_argument&) { traitRejected=true; }
        try { app->setOverrideGlassTintAmount(invalid); } catch(const std::invalid_argument&) { appRejected=true; }
        expect(traitRejected && appRejected && !app->overrideGlassTintAmount(),"invalid settings cannot corrupt the effective preference");
    }
    ctx->glassTintAmount=.75f;
    auto window=new_shared<UIWindow>();auto controller=new_shared<ProbeController>();
    window->setRootViewController(controller);window->makeKeyAndVisible();
    auto mask=new_shared<ProbeView>();controller->leaf->setMask(mask);
    auto glass=new_shared<BackdropEffectView>();glass->setFrame({0,0,120,64});controller->root->addSubview(glass);
    const auto before=window->traitCollection();
    expect(before->glassTintAmount()==.75f && controller->leaf->traitCollection()->glassTintAmount()==.75f,
           "initial views and controllers inherit the system preference before display");
    app->setOverrideGlassTintAmount(.1f);
    expect(app->glassTintAmount()==.1f && applicationRunLoop(),"an app setting overrides the backend before the next layout");
    expect(controller->traitCollection()->glassTintAmount()==.1f && controller->leaf->traitCollection()->glassTintAmount()==.1f
        && mask->traitCollection()->glassTintAmount()==.1f && glass->traitCollection()->glassTintAmount()==.1f,
           "the application override propagates through controllers, ordinary views, masks and glass");
    const auto materialFill=std::static_pointer_cast<CABackdropEffectLayer>(glass->layer())->effect().uniform("nativeFillOpacity");
    expect(materialFill && !materialFill->empty() && std::abs(materialFill->front()-.1f)<.00001f,
           "an existing glass view invalidates its resolved material when only the preference changes");
    expect(before->glassTintAmount()==.75f && controller->previous->glassTintAmount()==.75f,
           "previous trait snapshots remain immutable after changing the preference");
    auto late=new_shared<ProbeController>();controller->addChild(late);late->loadViewIfNeeded();
    expect(late->leaf->traitCollection()->glassTintAmount()==.1f,"late children inherit an existing application override");
    ctx->glassTintAmount=1;const auto notifications=controller->notifications;
    expect(applicationRunLoop() && controller->notifications==notifications,"system changes cannot overwrite an explicit app setting");
    app->setOverrideGlassTintAmount(std::nullopt);
    expect(applicationRunLoop() && controller->leaf->traitCollection()->glassTintAmount()==1,"clearing the override follows the latest system value");
    ctx->glassTintAmount=0;
    expect(applicationRunLoop() && controller->leaf->traitCollection()->glassTintAmount()==0,"live system changes propagate without a resize event");
    ctx->glassTintAmount=std::numeric_limits<float>::quiet_NaN();
    expect(applicationRunLoop() && controller->leaf->traitCollection()->glassTintAmount()==.5f,"an unavailable system value safely restores the original material");
    auto other=new_shared<InheritedRoot>(UITraitCollection::traitCollectionWithGlassTintAmount(.9f));other->addSubview(controller->leaf);
    expect(controller->leaf->traitCollection()->glassTintAmount()==.9f,"reparenting adopts the new environment's glass preference");
    window->setRootViewController(nullptr);ctx->glassTintAmount=.5f;
}
}
int main() {
    auto previousContext=SkiaCtx::_main;auto previousApplication=UIApplication::shared;auto previousCurrent=UITraitCollection::current();
    auto ctx=std::make_shared<Context>();SkiaCtx::_main=ctx;UIApplication::shared=new_shared<UIApplication>();
    expect(SDL_InitSubSystem(SDL_INIT_EVENTS),"event subsystem initializes for frame integration tests");
    testPolicy(*ctx);testInheritance(ctx);testRunLoop(ctx);testGlass(ctx);testGlassPreference(ctx);
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
    UIApplication::shared=previousApplication;SkiaCtx::_main=previousContext;UITraitCollection::setCurrent(previousCurrent);
    if (failures) return 1;
    std::cout<<"Size-class policy and trait propagation tests passed\n";
}
