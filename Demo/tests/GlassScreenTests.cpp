#include "../../UIKitCore/lib/platforms/apple/AppleSystemFont.hpp"
#include "../../Demo/app/Screens/GlassDemoViewController/GlassScreenViewController.hpp"
#include "../../Demo/app/Screens/GlassDemoViewController/DemoSystemSymbol.hpp"
#include <NXNavigationController.h>
#include <NXNavigationItem.h>
#include "UITabBarViewInternal.hpp"
#include "UINavigationBarInternal.hpp"
#include <UIApplication.h>
#include <UIEvent.h>
#include <UITouch.h>
#include <SkiaCtx.h>
#include <SDL3/SDL.h>
#include "include/core/SkSurface.h"
#include "include/core/SkBitmap.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>
#include <array>
using namespace NXKit;
namespace NXKit {
class LensViewTestHarness {
public:
    static void advance(const std::shared_ptr<LensView>& lens) {lens->advance(1.0/120);}
};
class UIGestureRecognizerTestHarness {
public:
    static void pull(const std::shared_ptr<UIWindow>& window,NXPoint start,NXFloat distance,const std::function<void()>& beforeRelease) {
        static unsigned long id=6000;
        auto touch=new_shared<UITouch>(++id,start,Timer()); auto event=new_shared<UIEvent>();
        event->_allTouches.insert(touch); touch->_window=window;
        auto send=[&](UITouchPhase phase,NXFloat delta,double time) {
            touch->updateAbsoluteLocation(start+NXPoint(0,delta));
            touch->_timestamp=Timer(time); touch->_phase=phase; window->sendEvent(event);
        };
        send(UITouchPhase::began,0,0);
        send(UITouchPhase::moved,distance*.25f,16);
        send(UITouchPhase::moved,distance*.5f,24);
        send(UITouchPhase::moved,distance,32);
        beforeRelease();
        send(UITouchPhase::ended,distance,48);
    }
    static void tap(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<UIControl>& control) {
        static unsigned long id=5000;
        const auto point=control->convertToView({control->bounds().width()*.5f,control->bounds().height()*.5f},window);
        auto touch=new_shared<UITouch>(++id,point,Timer()); auto event=new_shared<UIEvent>();
        event->_allTouches.insert(touch); touch->_window=window;
        touch->_phase=UITouchPhase::began; window->sendEvent(event);
        touch->_timestamp=Timer(80); touch->_phase=UITouchPhase::ended; window->sendEvent(event);
    }
};
bool applicationRunLoop();
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>) { return true; }
}
namespace {
class Context final : public SkiaCtx {
public:
    NXKit::NXSize size{402,874}; UIEdgeInsets safe{62,0,34,0}; bool dark=false; float density=2;
    Context() { fontMgr=SkFontMgr_New_CoreText(nullptr); }
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style = SkFontStyle(), NXKit::NXFloat size = 17) const override { return Internal::appleSystemTypeface(style,size); }
    NXKit::NXSize getSize() override { return size; }
    UIEdgeInsets deviceSafeAreaInsets() override { return safe; }
    UIUserInterfaceStyle getThemeMode() override { return dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light; }
    float getScaleFactor() override { return density; }
    sk_sp<SkSurface> getBackbufferSurface() override { return nullptr; }
    void swapBuffers() override {}
    void sleepForNextFrame() override {}
    bool platformRunLoop(std::function<bool()>) override { return false; }
protected:
    skgpu::graphite::Context* graphiteContext() override { return nullptr; }
    skgpu::graphite::Recorder* graphiteRecorder() override { return nullptr; }
};
void frame() { for(int i=0;i<4;++i) applicationRunLoop(); }
void settle() { for(int i=0;i<4;++i) { const auto layers=UIView::layersWithAnimations; for(const auto& layer:layers)layer->animateAt(Timer(100000)); frame(); } }
void snapshot(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<Context>& ctx,const std::string& path) {
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(ctx->size.width*ctx->density,ctx->size.height*ctx->density));surface->getCanvas()->scale(ctx->density,ctx->density);surface->getCanvas()->clear(SK_ColorBLACK);
    UITraitCollection::setCurrent(window->traitCollection());window->drawAndLayoutTreeIfNeeded();window->layer()->skiaRender(surface->getCanvas());
    SkBitmap bitmap;bitmap.allocN32Pixels(ctx->size.width*ctx->density,ctx->size.height*ctx->density);surface->readPixels(bitmap,0,0);
    std::ofstream output(path,std::ios::binary);output<<"P6\n"<<bitmap.width()<<' '<<bitmap.height()<<"\n255\n";
    for(int y=0;y<bitmap.height();++y)for(int x=0;x<bitmap.width();++x){const auto c=bitmap.getColor(x,y);const char rgb[]={char(SkColorGetR(c)),char(SkColorGetG(c)),char(SkColorGetB(c))};output.write(rgb,3);}
}
}
int main(int argc,char**argv) {
    auto ctx=std::make_shared<Context>(); SkiaCtx::_main=ctx;
    UIApplication::shared=new_shared<UIApplication>(); UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>(); SDL_InitSubSystem(SDL_INIT_EVENTS);
    auto window=new_shared<UIWindow>(); window->setFrame({NXKit::NXPoint(),ctx->size});
    auto controller=new_shared<GlassScreenViewController>();
    if(argc==4&&std::string(argv[1])=="--navigation") {
        ctx->density=3;window->setRootViewController(controller);window->makeKeyAndVisible();frame();
        controller->selectedNavigationController()->pushViewController(new_shared<GlassNavigationDemoViewController>(std::stoul(argv[3])),false);settle();frame();snapshot(window,ctx,argv[2]);
    } else if(argc>1) {
        if(argc!=16&&argc!=17) { std::cerr<<"OUTPUT W H TOP LEFT BOTTOM RIGHT DARK TAB ACCENT BACKDROP DETAILS ADDED REVERSED SCROLL [SCALE]\n";return 2; }
        if(argc==17)ctx->density=std::stoi(argv[16]);
        if(ctx->density<1||ctx->density>3)return 2;
        ctx->size={std::stof(argv[2]),std::stof(argv[3])}; ctx->safe={std::stof(argv[4]),std::stof(argv[5]),std::stof(argv[6]),std::stof(argv[7])};ctx->dark=std::stoi(argv[8]);
        window->setRootViewController(controller); window->makeKeyAndVisible();frame();
        controller->setSelectedIndex(std::stoi(argv[9]));auto screen=controller->selectedList();
        for(int i=0;i<std::stoi(argv[10]);++i)screen->cycleAccent();for(int i=0;i<std::stoi(argv[11]);++i)screen->cycleBackdrop();
        if(std::stoi(argv[12]))screen->toggleDetails();for(int i=0;i<std::stoi(argv[13]);++i)screen->addItem();if(std::stoi(argv[14]))screen->toggleOrder();
        frame(); screen->setScrollPosition(std::stof(argv[15]));frame();
        auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(ctx->size.width*ctx->density,ctx->size.height*ctx->density));surface->getCanvas()->scale(ctx->density,ctx->density);surface->getCanvas()->clear(SK_ColorBLACK);
        UITraitCollection::setCurrent(window->traitCollection());window->layer()->skiaRender(surface->getCanvas());
        SkBitmap b;b.allocN32Pixels(ctx->size.width*ctx->density,ctx->size.height*ctx->density);surface->readPixels(b,0,0);
        std::ofstream f(argv[1],std::ios::binary);f<<"P6\n"<<b.width()<<" "<<b.height()<<"\n255\n";
        for(int y=0;y<b.height();++y)for(int x=0;x<b.width();++x){auto c=b.getColor(x,y);unsigned char rgb[]={static_cast<unsigned char>(SkColorGetR(c)),static_cast<unsigned char>(SkColorGetG(c)),static_cast<unsigned char>(SkColorGetB(c))};f.write(reinterpret_cast<char*>(rgb),3);}
        auto nav=controller->selectedNavigationController()->navigationBar()->frame(),tab=controller->tabBar()->frame();
        const auto lens=Internal::TabBarViewAccess::state(*controller->tabBar())._lens->frame();
        std::ofstream m(std::string(argv[1])+".json");m<<"{\"navigation\":["<<nav.minX()<<','<<nav.minY()<<','<<nav.width()<<','<<nav.height()<<"],\"tab\":["<<tab.minX()<<','<<tab.minY()<<','<<tab.width()<<','<<tab.height()<<"],\"lens\":["<<tab.minX()+lens.minX()<<','<<tab.minY()+lens.minY()<<','<<lens.width()<<','<<lens.height()<<"],\"rows\":[";
        int rowIndex=0;
        for(const auto* id:{"background","accent","add-row","sort","reset"}) {
            auto control=screen->action(id);const auto point=control->convertToView({},screen);
            if(rowIndex++)m<<',';m<<'['<<point.x<<','<<point.y<<','<<control->bounds().width()<<','<<control->bounds().height()<<']';
        }
        m<<"]}\n";
    } else {
        int failures=0;auto expect=[&](bool value,const char* message){if(!value){++failures;std::cerr<<"FAIL: "<<message<<'\n';}};
        auto root=new_shared<UIViewController>();auto nav=new_shared<NXNavigationController>(root);window->setRootViewController(nav);window->makeKeyAndVisible();frame();
        auto originalInsets=nav->additionalSafeAreaInsets();
        auto rootItem=nav->topNavigationItem();
        expect(!rootItem->hidesNavigationBar()&&!rootItem->hidesToolbar(),"destination chrome defaults visible");
        rootItem->setHidesNavigationBar(true);frame();
        expect(nav->additionalSafeAreaInsets().top==0&&nav->additionalSafeAreaInsets().bottom==originalInsets.bottom,"navigation visibility updates live without hiding the toolbar reservation");
        rootItem->setHidesNavigationBar(false);rootItem->setHidesToolbar(true);frame();
        expect(nav->additionalSafeAreaInsets().top==originalInsets.top&&nav->additionalSafeAreaInsets().bottom==0,"toolbar visibility updates independently");
        rootItem->setHidesToolbar(false);frame();
        nav->pushViewController(controller,false);frame();
        auto screen=controller->selectedList();
        for(const auto style:{UIScrollEdgeEffectStyle::soft,UIScrollEdgeEffectStyle::hard,UIScrollEdgeEffectStyle::none,UIScrollEdgeEffectStyle::automatic}) {
            screen->cycleScrollEdgeStyle();frame();
            expect(screen->scrollView()->topEdgeEffect()->style()==style
                &&screen->scrollView()->bottomEdgeEffect()->style()==style,
                "Demo scroll-edge setting cycles through soft, hard, none and back to automatic");
        }
        {
            auto holder=new_shared<UIView>();screen->addSubview(holder);
            auto widget=new_shared<NXNavigationActionsView>();holder->addSubview(widget);
            int queries=0;widget->setActionProvider([&] {++queries;return std::vector<NXResponderAction>{};});
            queries=0;applicationRunLoop();
            expect(queries==1,"visible navigation widgets refresh their responder actions");
            holder->setHidden(true);queries=0;applicationRunLoop();
            expect(queries==0,"hidden ancestor stops navigation widget polling");
            holder->setHidden(false);applicationRunLoop();
            expect(queries==1,"revealing chrome resumes responder action updates");
            widget->removeFromSuperview();queries=0;applicationRunLoop();
            expect(queries==0,"detached chrome stops responder-chain polling");
            holder->removeFromSuperview();
        }
        expect(nav->topNavigationItem()->hidesNavigationBar()&&nav->topNavigationItem()->hidesToolbar(),"full screen destination hides both NX bars through its own item");
        expect(nav->additionalSafeAreaInsets()==UIEdgeInsets::zero&&screen->safeAreaInsets().top==ctx->safe.top+54,"UIKit navigation reserves its native inline bar above the list");
        expect(controller->viewControllers().size()==4,"four real navigation stacks are mounted in UIKit tabs");
        auto innerNav=controller->selectedNavigationController();
        expect(screen->superview().lock()&&innerNav->topViewController()->navigationController()==innerNav,"the list is the navigation root with real containment");
        expect(innerNav->navigationBar()->items().size()==1&&controller->tabBar()->items().size()==4,"bars derive their items from the real controller hierarchy");
        expect(controller->tabBar()->frame()==NXRect(21,791,360,62),
               "production floating platter matches the retained native 402x874 phone capture");
        const auto initialLens=Internal::TabBarViewAccess::state(*controller->tabBar())._lens->frame();
        expect(initialLens==NXRect(4,4,95,54),"production lens matches the native inset and size");
        expect(screen->safeAreaInsets().bottom==83,"the tab reservation includes the device safe area once");
        auto symbol2=UIImage::systemImageNamed("chevron.left",20,600,2);
        auto symbol3=UIImage::systemImageNamed("chevron.left",20,600,3);
        expect(symbol2&&symbol3&&symbol2->scale()==2&&symbol3->scale()==3&&symbol3->cgImage()->size().height>symbol2->cgImage()->size().height,"symbol raster density follows the requested display scale");
        expect(!UIImage::systemImageNamed("nxkit.nonexistent.symbol")&&!UIImage::systemImageNamed(""),"missing symbols return null for caller fallbacks");
        bool rejectedSymbol=false;
        try { UIImage::systemImageNamed("plus",20,400,0); } catch(const std::invalid_argument&) { rejectedSymbol=true; }
        expect(rejectedSymbol,"invalid symbol raster density is rejected");
        expect(screen->scrollView()->bounceVertically()&&!screen->scrollView()->bounceHorizontally(),"full-screen list enables vertical spring bounce only");
        for(const bool bottom:{false,true}) {
            screen->setScrollPosition(bottom?10000:0);frame();
            const auto rest=screen->scrollView()->contentOffset();
            const auto navigation=controller->selectedNavigationController()->navigationBar()->frame(),tab=controller->tabBar()->frame();
            NXFloat pulled=0;
            UIGestureRecognizerTestHarness::pull(window,{200,400},bottom?-120:120,[&] {
                pulled=screen->scrollView()->contentOffset().y-rest.y;
                expect(bottom?pulled>0:pulled<0,"drag stretches content beyond the corresponding edge");
                expect(std::abs(pulled)<120,"edge pull applies rubber-band resistance");
            });
            expect(screen->scrollView()->isDecelerating(),"release starts animated spring recovery");
            const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
            while(screen->scrollView()->isDecelerating()&&std::chrono::steady_clock::now()<deadline) {
                std::this_thread::sleep_for(std::chrono::milliseconds(8));frame();
            }
            expect(!screen->scrollView()->isDecelerating()&&std::abs(screen->scrollView()->contentOffset().y-rest.y)<.5f,"spring settles back at the content edge");
            expect(controller->selectedNavigationController()->navigationBar()->frame()==navigation&&controller->tabBar()->frame()==tab,"navigation and tab glass stay fixed throughout edge bouncing");
        }
        screen->setScrollPosition(0);frame();
        Internal::TabBarViewAccess::state(*controller->tabBar())._controls[3]->performPrimaryAction();screen=controller->selectedList();
        expect(screen->selectedTab()==3,"fourth tab selects its own root content controller");settle();frame();
        innerNav=controller->selectedNavigationController();
        auto& barState=Internal::NavigationBarAccess::state(*innerNav->navigationBar());
        expect(barState.right.backgrounds.size()==2&&
               barState.right.backgrounds[0]->frame().minX()-barState.right.backgrounds[1]->frame().maxX()>=7.99f,
               "prominent and plain trailing items keep separate glass with an eight-point gap");
        UIGestureRecognizerTestHarness::tap(window,barState.right.buttons[0]->control);
        screen->action("add-row")->performPrimaryAction();expect(screen->addedItems()==2,"bar and list add actions change collection contents");
        barState.right.buttons[1]->control->performPrimaryAction();expect(screen->showsDetails(),"more changes the actual row details");
        screen->action("sort")->performPrimaryAction();expect(screen->reversed(),"sort reorders the collection");
        screen->action("item-2")->performPrimaryAction();expect(screen->completedItems()==1,"item actions update completion state");
        screen->action("accent")->performPrimaryAction();screen->action("background")->performPrimaryAction();expect(screen->accent()==1&&screen->backdrop()==1,"appearance rows change actual colors");
        controller->setSelectedIndex(1);expect(controller->selectedList()!=screen,"tabs own independent content controllers");controller->setSelectedIndex(3);expect(controller->selectedList()==screen&&screen->addedItems()==2&&screen->completedItems()==1,"tab changes preserve each root list and its state");
        frame();screen->setScrollPosition(0);frame();
        UIGestureRecognizerTestHarness::tap(window,screen->action("navigation-demo"));
        expect(Internal::NavigationBarAccess::state(*innerNav->navigationBar()).right.backgrounds.front()->layer()->presentation()!=nullptr,"playground entry animates the real navigation glass");
        const std::array<size_t,GlassNavigationDemoViewController::arrangementCount> expectedGlass={1,2,2,1,2,1,2,1};
        std::shared_ptr<GlassNavigationDemoViewController> destination;
        for(size_t i=0;i<expectedGlass.size();++i) {
            settle();frame();destination=std::dynamic_pointer_cast<GlassNavigationDemoViewController>(innerNav->topViewController());
            expect(destination&&destination->arrangement()==i&&innerNav->viewControllers().size()==i+2,"navigation playground pushes each arrangement onto the real tab stack");
            if(!destination)break;
            auto& chrome=Internal::NavigationBarAccess::state(*innerNav->navigationBar());
            expect(chrome.right.backgrounds.size()==expectedGlass[i],"playground shared/prominent/spaced/text/unshared/hidden configurations use the expected glass groups");
            expect(chrome.left.groups.size()==(i==5?2:1)&&chrome.left.buttons.front()->back,"playground retains automatic Back and supplements it in the leading-items case");
            expect(destination->navigationController()==innerNav&&destination->tabBarController()==controller,"pushed playground screens retain navigation and tab containment");
            if(i==0){chrome.right.buttons.front()->control->performPrimaryAction();expect(screen->addedItems()==2,"matched playground items replace collection actions rather than keeping stale handlers");}
            if(i+1<expectedGlass.size())destination->action("next")->performPrimaryAction();
        }
        controller->setSelectedIndex(1);controller->setSelectedIndex(3);frame();
        expect(innerNav->topViewController()==destination,"switching tabs preserves the pushed morphing stack");
        for(size_t i=expectedGlass.size();i>0;--i) {
            Internal::NavigationBarAccess::state(*innerNav->navigationBar()).left.buttons.front()->control->performPrimaryAction();settle();frame();
            expect(innerNav->viewControllers().size()==i,"automatic Back reverses each playground push");
        }
        expect(controller->selectedList()==screen&&screen->addedItems()==2&&screen->completedItems()==1&&screen->accent()==1,"returning from morphing screens preserves the original collection and appearance");
        screen->action("navigation-demo")->performPrimaryAction();settle();frame();
        destination=std::dynamic_pointer_cast<GlassNavigationDemoViewController>(innerNav->topViewController());
        destination->action("arrangement-5")->performPrimaryAction();settle();frame();
        destination=std::dynamic_pointer_cast<GlassNavigationDemoViewController>(innerNav->topViewController());
        expect(destination->arrangement()==5,"playground links can push arrangements in any order");
        destination->action("collection")->performPrimaryAction();settle();frame();
        expect(innerNav->viewControllers().size()==1&&controller->selectedList()==screen,"Pop to collection removes all playground destinations without leaving the fullscreen demo");
        screen->action("large-title-demo")->performPrimaryAction();settle();frame();
        auto large=innerNav->topViewController();auto largeScroll=std::dynamic_pointer_cast<UIScrollView>(large->view());
        expect(largeScroll&&innerNav->navigationBar()->frame().height()==106,"large-title Demo mounts a real expanded navigation accessory");
        if(largeScroll) {
            const auto top=largeScroll->adjustedContentInset().top;
            largeScroll->setContentOffset({0,-top+80},false);frame();
            expect(innerNav->navigationBar()->frame().height()==54,"Demo scroll collapses the large title");
            largeScroll->setContentOffset({0,-top},false);frame();
            auto never=std::dynamic_pointer_cast<UIControl>(largeScroll->subviews().at(2));never->performPrimaryAction();settle();frame();
            expect(innerNav->navigationBar()->frame().height()==54,"Demo Never destination keeps the inline title");
            innerNav->popViewController(false);frame();
            expect(innerNav->navigationBar()->frame().height()==106,"returning to the large-title Demo restores its own extent");
            ctx->size={852,393};ctx->safe={0,62,21,62};frame();
            const auto insets=largeScroll->adjustedContentInset();
            expect(insets.left==0&&insets.right==0&&largeScroll->contentSize().width<=largeScroll->bounds().width(),"landscape large-title Demo has no horizontal scroll range from safe-area insets");
            expect(insets.top==largeScroll->safeAreaInsets().top&&insets.top>0,"landscape large-title Demo retains vertical navigation insets");
            ctx->size={402,874};ctx->safe={62,0,34,0};frame();
        }
        innerNav->popToRootViewController(false);frame();
        frame();screen->setScrollPosition(10000);frame();
        expect(screen->scrollView()->contentOffset().y>0,"grouped content scrolls beyond the first viewport");
        expect(controller->tabBar()->frame().maxY()==ctx->size.height-21,"floating tab platter overlaps the home safe area by the native thirteen points");
        ctx->size={852,393};ctx->safe={0,62,21,62};frame();
        expect(controller->tabBar()->frame().minX()>=ctx->safe.left&&controller->tabBar()->frame().maxX()<=ctx->size.width-ctx->safe.right,"rotation keeps the tab platter within horizontal safe areas");
        const auto scroll=screen->scrollView();const auto insets=scroll->adjustedContentInset();
        expect(insets.left==0&&insets.right==0&&scroll->contentSize().width<=scroll->bounds().width(),"landscape fullscreen list has no horizontal scroll range from safe-area insets");
        expect(insets.top==scroll->safeAreaInsets().top&&insets.top>0,"landscape fullscreen list retains vertical navigation insets");
        screen->action("reset")->performPrimaryAction();expect(!screen->showsDetails()&&!screen->reversed()&&screen->addedItems()==0&&screen->completedItems()==0,"reset restores actual screen state");
        UIGestureRecognizerTestHarness::tap(window,Internal::NavigationBarAccess::state(*controller->selectedNavigationController()->navigationBar()).left.buttons[0]->control);settle();expect(nav->topViewController()==root&&nav->viewControllers().size()==1,"custom back really pops the navigation stack");
        expect(nav->additionalSafeAreaInsets()==originalInsets,"popping restores the previous destination's NX chrome and safe areas");
        window->setRootViewController(nullptr);if(failures)return 1;std::cout<<"Full-screen glass tests passed\n";
    }
    window->setRootViewController(nullptr);UIApplication::shared.reset();SkiaCtx::_main.reset();SDL_QuitSubSystem(SDL_INIT_EVENTS);
}
