#include <UIKit.h>
#include <UIApplication.h>
#include <SkiaCtx.h>
#include <UIWindow.h>
#include <UIEvent.h>
#include <UITouch.h>
#include "UITabBarViewInternal.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#ifdef __APPLE__
#include "include/ports/SkFontMgr_mac_ct.h"
#include "platforms/apple/AppleSystemFont.hpp"
#endif
#include <iostream>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <fstream>
#include <filesystem>

using namespace NXKit;
namespace NXKit {
void renderApplicationFrame();
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>) { return true; }
class DispatchQueueTestHarness { public: static void pump() { DispatchQueue::main()->performAll(); } };
class LensViewTestHarness { public: static void advance(const std::shared_ptr<LensView>& lens,double seconds) {lens->advance(seconds);} };
class UIGestureRecognizerTestHarness {
public:
    static auto event(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<UITouch>& touch) {
        auto event=new_shared<UIEvent>(); event->_allTouches.insert(touch); touch->_window=window; return event;
    }
    static void send(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<UIEvent>& event,
                     const std::shared_ptr<UITouch>& touch,UITouchPhase phase,NXPoint point,double time) {
        touch->updateAbsoluteLocation(point);touch->_phase=phase;touch->_timestamp=Timer(time);window->sendEvent(event);
    }
};
}
namespace {
int failures=0;
void expect(bool value,const char* message) {if(!value){++failures;std::cerr<<"FAIL: "<<message<<'\n';}}
template<class F> bool rejects(F action) {try{action();return false;}catch(const std::invalid_argument&){return true;}catch(const std::logic_error&){return true;}}
class Context final:public SkiaCtx {
public:
    NXKit::NXSize size{390,844};float scale=1;bool dark=false;UIEdgeInsets safe{44,0,34,0};
    Context() {
#ifdef __APPLE__
        fontMgr=SkFontMgr_New_CoreText(nullptr);
#endif
    }
#ifdef __APPLE__
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style=SkFontStyle(),NXFloat size=17)const override{return Internal::appleSystemTypeface(style,size);}
#endif
    NXKit::NXSize getSize() override{return size;}
    float getScaleFactor() override{return scale;}
    UIEdgeInsets deviceSafeAreaInsets() override{return safe;}
    UIUserInterfaceStyle getThemeMode() override{return dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;}
    sk_sp<SkSurface> getBackbufferSurface() override{return nullptr;}
    void swapBuffers() override{}
    bool platformRunLoop(std::function<bool()>) override{return false;}
protected:
    skgpu::graphite::Context* graphiteContext() override{return nullptr;}
    skgpu::graphite::Recorder* graphiteRecorder() override{return nullptr;}
};
class Screen final:public UIViewController {
public:
    int loads=0,willAppear=0,didAppear=0,willDisappear=0,didDisappear=0;
    int value=0;bool appearedUnloaded=false;std::function<void()> onDisappear;
    void loadView() override {++loads;setView(new_shared<UIView>());}
    void viewWillAppear(bool) override {++willAppear;appearedUnloaded|=!viewIsLoaded();}
    void viewDidAppear(bool) override {++didAppear;}
    void viewWillDisappear(bool) override {++willDisappear;const auto callback=onDisappear;if(callback)callback();}
    void viewDidDisappear(bool) override {++didDisappear;}
};
class Delegate final:public UITabBarControllerDelegate {
public:
    int should=0,did=0;bool allow=true;
    std::function<void()> onShould,onDid;
    bool tabBarControllerShouldSelect(const std::shared_ptr<UITabBarController>&,
                                      const std::shared_ptr<UIViewController>&) override {
        ++should;if(onShould)onShould();return allow;
    }
    void tabBarControllerDidSelect(const std::shared_ptr<UITabBarController>&,
                                   const std::shared_ptr<UIViewController>&) override {++did;if(onDid)onDid();}
};
class BarDelegate final:public UITabBarDelegate {
public:int did=0;
    void tabBarDidSelect(const std::shared_ptr<UITabBar>&,const std::shared_ptr<UITabBarItem>&) override{++did;}
};
// Explicit frames keep geometry checks independent of the render deadline.
void frame() {for(int i=0;i<4;++i){DispatchQueueTestHarness::pump();renderApplicationFrame();DispatchQueueTestHarness::pump();}}
void settle() {UIView::completePendingAnimations();DispatchQueueTestHarness::pump();}
void settleTabLift(const std::shared_ptr<UITabBarView>& bar) {
    // Transit, the containing control's hold and the shared lens lifetime have
    // independent clocks. Advance both live clocks before asserting rest.
    UIView::completePendingAnimations();
    const auto lens=Internal::TabBarViewAccess::state(*bar)._lens;
    for(int i=0;i<240;++i) {
        Internal::TabBarViewAccess::advance(*bar,1./120);
        LensViewTestHarness::advance(lens,1./120);
    }
    settle();frame();
}
auto windowFor(const std::shared_ptr<UIViewController>& controller) {
    auto window=new_shared<UIWindow>();UIApplication::shared->delegate->window=window;
    window->setRootViewController(controller);window->makeKeyAndVisible();frame();return window;
}
void tap(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<UITabBarView>& bar,size_t index) {
    auto& state=Internal::TabBarViewAccess::state(*bar);
    const auto control=state._controls[index];
    const auto p=control->convertToView({control->bounds().width()*.5f,control->bounds().height()*.5f},nullptr);
    auto touch=new_shared<UITouch>(1,p,Timer(1000));
    auto event=UIGestureRecognizerTestHarness::event(window,touch);
    UIGestureRecognizerTestHarness::send(window,event,touch,UITouchPhase::began,p,1000);
    UIGestureRecognizerTestHarness::send(window,event,touch,UITouchPhase::ended,p,1050);
    settle();
}
void barCases(const std::shared_ptr<Context>& ctx) {
    auto root=new_shared<UIViewController>();auto window=windowFor(root);
    auto bar=new_shared<UITabBarView>(NXRect(16,150,358,62));root->view()->addSubview(bar);
    expect(bar->items().empty()&&!bar->selectedItem(),"standalone bar begins empty and unselected");
    auto a=std::make_shared<UITabBarItem>("One",nullptr,1);
    auto b=std::make_shared<UITabBarItem>("Two",nullptr,2);
    bar->setItems({a,b});frame();
    expect(!bar->selectedItem(),"assigning standalone items does not synthesize a selection");
    auto delegate=std::make_shared<BarDelegate>();bar->setDelegate(delegate);
    bar->setSelectedItem(b);expect(delegate->did==0,"programmatic bar selection is silent");
    tap(window,bar,0);expect(bar->selectedItem()==a&&delegate->did==1,"routed touch commits a bar item and notifies once");
    tap(window,bar,0);expect(delegate->did==2,"user reselecting a bar item still notifies its delegate");
    b->setEnabled(false);tap(window,bar,1);
    expect(bar->selectedItem()==a&&delegate->did==2,"disabled items do not activate or notify");
    expect(!Internal::TabBarViewAccess::state(*bar)._controls[1]->canBecomeFocused(),"disabled items are excluded from focus");
    b->setEnabled(true);b->setTitle("Renamed");b->setBadgeValue("12");b->setBadgeColor(UIColor::systemGreen);
    auto& state=Internal::TabBarViewAccess::state(*bar);
    expect(state._normalLabels[1]->text()=="Renamed"&&!state.badges[1]->isHidden()
           &&state.badgeLabels[1]->text()=="12", "live titles and badges invalidate the installed bar");
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(8,8));surface->getCanvas()->clear(SK_ColorRED);
    auto image=std::make_shared<UIImage>(std::make_shared<NXKit::CGImage>(surface->makeImageSnapshot()),1);
    surface->getCanvas()->clear(SK_ColorBLUE);
    auto selected=std::make_shared<UIImage>(std::make_shared<NXKit::CGImage>(surface->makeImageSnapshot()),1);
    b->setImage(image);b->setSelectedImage(selected);
    expect(std::dynamic_pointer_cast<UIImageView>(state._normalSymbols[1])->image()==image
           &&std::dynamic_pointer_cast<UIImageView>(state._selectedSymbols[1])->image()==selected,
           "normal and selected item images have independent live sources");
    bar->setItems({b,a},true);expect(bar->selectedItem()==a,"item reorder preserves selection by identity");settle();
    expect(rejects([&]{bar->setItems({a,a});})&&bar->items().size()==2,"duplicate items are rejected atomically");
    expect(rejects([&]{bar->setSelectedItem(std::make_shared<UITabBarItem>());}),"foreign selected items are rejected");
    bar->setSelectedItem(nullptr);expect(!bar->selectedItem()&&state._lens->isHidden(),"nil selection clears the selection surface");
    for(float scale:{1.f,2.f,3.f})for(bool dark:{false,true}) {
        ctx->scale=scale;ctx->dark=dark;frame();bar->setItems({b});bar->setSelectedItem(b);bar->layoutIfNeeded();
        expect(bar->traitCollection()->displayScale()==scale,"bar follows inherited display scale");
        auto canvas=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(int(360*scale),int(64*scale)));
        canvas->getCanvas()->clear(SK_ColorWHITE);canvas->getCanvas()->scale(scale,scale);
        canvas->getCanvas()->translate(-bar->frame().minX(),-bar->frame().minY());
        bar->drawAndLayoutTreeIfNeeded();bar->layer()->skiaRender(canvas->getCanvas());
        expect(state._lens->frame().width()>0&&state._lens->frame().height()>0,"one-item material renders across scales and themes");
        SkBitmap pixels;pixels.allocN32Pixels(int(360*scale),int(64*scale));canvas->readPixels(pixels,0,0);
        int ink=0;
        for(int y=0;y<pixels.height();++y)for(int x=0;x<pixels.width();++x)
            ink+=pixels.getColor(x,y)!=SK_ColorWHITE;
        expect(ink>100,"rendered tab material and item content produce visible pixels");
    }
    ctx->scale=1;ctx->dark=false;
    auto c=std::make_shared<UITabBarItem>("Three",nullptr,3);
    auto d=std::make_shared<UITabBarItem>("Four",nullptr,4);
    bar->setItems({a,b,c,d});bar->setFrame({0,150,1000,62});bar->setSelectedItem(d);bar->layoutIfNeeded();
    const auto control=state._controls[3];
    const auto label=state._normalLabels[3];
    const auto labelCenter=label->convertToView({label->bounds().width()*.5f,7},bar);
    expect(std::abs(state._lens->frame().midX()-labelCenter.x)<.01f
           &&std::abs(control->frame().midX()-labelCenter.x)<.01f,
           "wide tab layout keeps selection, caption and touch cell centers aligned");
    for(const NXFloat width:{360.f,388.f,520.f,1000.f})for(const auto index:{size_t(0),size_t(3)}) {
        bar->setFrame({0,150,width,62});bar->setSelectedItem(index==0?a:d);bar->layoutIfNeeded();
        const auto lens=state._lens->frame();
        const auto edge=index==0?lens.minX():width-lens.maxX();
        expect(edge>=3.99f&&(width>388||std::abs(edge-4)<.01f)
               &&lens.minY()==4&&lens.height()==54,
               "first and last selection lenses keep the native phone inset and fit wider platters");
        const auto caption=state._normalLabels[index];
        const auto center=caption->convertToView({caption->bounds().width()*.5f,0},bar);
        expect(std::abs(center.x-lens.midX())<.01f,"captions remain centered inside the inset selection lens");
    }
    bar->setItems({});bar->layoutIfNeeded();
    expect(!bar->selectedItem()&&!bar->point({5,5},nullptr),"empty bar has no selection or invisible hit surface");
    bar->setItems({a});bar->setFrame({0,0,0,0});bar->layoutIfNeeded();
    expect(std::isfinite(state._lens->frame().width()),"zero-size layout keeps finite selection geometry");
    window->setRootViewController(nullptr);UIApplication::shared->delegate->window.reset();
}
void controllerCases(const std::shared_ptr<Context>& ctx) {
    auto tabs=new_shared<UITabBarController>();
    auto a=new_shared<Screen>(),b=new_shared<Screen>(),c=new_shared<Screen>();
    a->setTitle("Alpha");b->setTitle("Beta");c->setTitle("Gamma");
    tabs->setViewControllers({a,b,c});
    expect(tabs->mode()==UITabBarControllerMode::automatic,"UIKit defaults to automatic mode");
    tabs->setMode(UITabBarControllerMode::tabBar);
    expect(rejects([&]{tabs->setMode(UITabBarControllerMode::tabSidebar);})
           &&tabs->mode()==UITabBarControllerMode::tabBar,"unimplemented sidebar rejects without changing the mode");
    tabs->setMode(UITabBarControllerMode::automatic);
    expect(a->tabBarController()==tabs&&a->loads==0&&b->loads==0&&!tabs->viewIsLoaded(),
           "configuration establishes tab containment without loading views");
    auto window=windowFor(tabs);auto bar=tabs->tabBar();
    expect(tabs->selectedIndex()==0&&tabs->selectedViewController()==a&&a->loads==1&&b->loads==0,
           "the controller selects and loads only its first content view");
    expect(a->willAppear==1&&a->didAppear==1&&b->willAppear==0,
           "initial appearance forwards once to selected content only");
    expect(bar->selectedItem()==a->tabBarItem()&&bar->delegate()==tabs,"bar selection and controller ownership agree");
    expect(rejects([&]{bar->setItems({});})&&rejects([&]{bar->setDelegate(nullptr);}),"managed item and delegate mutation are guarded");
    auto delegate=std::make_shared<Delegate>();tabs->setDelegate(delegate);
    tabs->setSelectedIndex(1);frame();
    expect(delegate->should==0&&delegate->did==0,"programmatic controller selection does not invoke user delegates");
    expect(a->willDisappear==1&&a->didDisappear==1&&b->willAppear==1&&b->didAppear==1,
           "selection balances appearance on the outgoing and incoming children");
    expect(!b->appearedUnloaded,"incoming content is loaded before its appearance callbacks");
    delegate->allow=false;tap(window,bar,2);
    expect(tabs->selectedViewController()==b&&delegate->should==1&&delegate->did==0,"delegate veto restores the committed selection");
    delegate->allow=true;tap(window,bar,2);
    expect(tabs->selectedViewController()==c&&delegate->did==1&&bar->selectedItem()==c->tabBarItem(),
           "routed user selection synchronizes content, bar and delegate");
    bar->setSelectedItem(a->tabBarItem());expect(tabs->selectedViewController()==a,"managed bar programmatic selection stays synchronized");
    a->setTitle("Updated");expect(bar->items()[0]->title()=="Updated","default tab item follows controller title changes");
    a->tabBarItem()->setTitle("Custom");a->setTitle("Other");
    expect(bar->items()[0]->title()=="Custom","an explicit tab title survives controller title updates");
    const auto replacement=std::make_shared<UITabBarItem>("Replacement",nullptr,8);
    a->setTabBarItem(replacement);expect(bar->items()[0]==replacement&&bar->selectedItem()==replacement,
                                      "replacing a selected controller's item updates the managed bar live");
    expect(rejects([&]{a->setTabBarItem(b->tabBarItem());})&&a->tabBarItem()==replacement,
           "sharing an item between contained controllers is rejected before mutation");
    a->setAdditionalSafeAreaInsets({0,0,7,0});frame();
    expect(a->view()->safeAreaInsets().bottom==90&&a->additionalSafeAreaInsets().bottom==7,
           "floating bar reserves content safe area without overwriting application insets");
    struct Placement {NXFloat safeBottom,gap,contentInset;};
    for(const auto expected:{Placement{0,8,70},Placement{21,8,70},Placement{34,21,83},Placement{60,47,109}}) {
        ctx->safe.bottom=expected.safeBottom;window->setNeedsLayout();frame();
        expect(bar->frame().maxY()==ctx->size.height-expected.gap
               &&a->view()->safeAreaInsets().bottom==expected.contentInset+7,
               "floating chrome and content reservations adapt without doubling the device inset");
    }
    ctx->safe.bottom=34;window->setNeedsLayout();frame();
    const auto barFrame=bar->frame();ctx->size={844,390};frame();
    expect(bar->frame().width()==barFrame.width()&&bar->frame().midX()==422&&bar->frame().maxY()==390-21,
           "three-tab mode retains its natural centered width when orientation changes");
    ctx->size={390,844};frame();
    expect(!tabs->setSelectedIndex(30)&&tabs->selectedViewController()==a,"invalid selection leaves state unchanged");
    expect(rejects([&]{tabs->setViewControllers({a,a});})&&tabs->viewControllers().size()==3,
           "duplicate controller list is rejected atomically");
    expect(rejects([&]{tabs->setViewControllers({tabs});}),"self containment is rejected");
    auto other=new_shared<UIViewController>();other->addChild(new_shared<UIViewController>());
    expect(rejects([&]{tabs->setViewControllers({other->children()[0]});}),"foreign containment is rejected");
    delegate->onShould=[&]{tabs->setViewControllers({a,b});};
    tap(window,bar,2);expect(tabs->selectedViewController()==a&&tabs->viewControllers().size()==2,
                           "delegate list mutation cannot commit a stale candidate");
    delegate->onShould=nullptr;tabs->setViewControllers({a,b,c});
    a->onDisappear=[&]{a->onDisappear=nullptr;tabs->setSelectedIndex(2);};
    tabs->setSelectedIndex(1);expect(tabs->selectedViewController()==c&&c->view()->superview().lock(),
                                   "selection requested in appearance callback drains after stable mutation");
    tabs->setSelectedIndex(1);tabs->setViewControllers({a,c});
    expect(tabs->selectedIndex()==1&&tabs->selectedViewController()==c&&b->parent().expired(),
           "replacing a removed selection uses the prior index and detaches removed content");
    c->value=42;tabs->setSelectedIndex(0);tabs->setSelectedIndex(1);
    expect(c->value==42&&c->loads==1,"switching tabs preserves controller identity and loaded content state");
    tabs->setViewControllers({});frame();
    expect(!tabs->selectedIndex()&&!tabs->selectedViewController()&&bar->isHidden()&&tabs->children().empty(),
           "clearing controllers removes content, selection, containment and bar obstruction");
    expect(a->additionalSafeAreaInsets().bottom==7,"removal preserves application safe-area configuration");
    window->setRootViewController(nullptr);UIApplication::shared->delegate->window.reset();
    std::weak_ptr<UITabBarController> weak=tabs;delegate->onDid=nullptr;delegate->onShould=nullptr;
    tabs.reset();expect(weak.expired()&&!bar->delegate(),"retaining a bar does not retain its controller or delegate");
}
void variantCases(const std::shared_ptr<Context>& ctx,const std::string& reportPath,const std::string& images) {
    std::ofstream report;if(!reportPath.empty())report.open(reportPath);
    if(report)report<<"{\"records\":[";bool comma=false;
    const char* titles[]={"Home","Favorites","Library","Settings","Search"};
    const char* symbols[]={"house.fill","star.fill","books.vertical.fill","slider.horizontal.3","magnifyingglass"};
    auto rect=[](std::ostream& out,NXRect r){out<<'['<<r.minX()<<','<<r.minY()<<','<<r.width()<<','<<r.height()<<']';};
    for(const NXFloat width:{320.f,360.f,390.f,402.f,430.f})for(bool dark:{false,true})for(size_t count:{2,3,4,5}) {
        ctx->size={width,874};ctx->safe={62,0,34,0};ctx->scale=3;ctx->dark=dark;
        auto tabs=new_shared<UITabBarController>();std::vector<std::shared_ptr<UIViewController>> controllers;
        for(size_t i=0;i<count;++i){auto c=new_shared<Screen>();c->setTitle(titles[i]);c->setTabBarItem(std::make_shared<UITabBarItem>(titles[i],UIImage::systemImageNamed(symbols[i],28,400,3),i));controllers.push_back(c);}
        tabs->setViewControllers(controllers);auto window=windowFor(tabs);auto bar=tabs->tabBar();
        const NXKit::NXPoint body{width*.5f,400};
        auto passive=new_shared<UITouch>(9000,body,Timer());auto passiveEvent=UIGestureRecognizerTestHarness::event(window,passive);
        UIGestureRecognizerTestHarness::send(window,passiveEvent,passive,UITouchPhase::began,body,0);
        UIGestureRecognizerTestHarness::send(window,passiveEvent,passive,UITouchPhase::ended,body,16);
        auto& state=Internal::TabBarViewAccess::state(*bar);
        const NXFloat platterWidth=count==2?188:(count==3?274:width-42);
        expect(std::abs(bar->frame().width()-platterWidth)<.01f&&bar->frame().midX()==width*.5f,
               "two and three tabs hug their content; four and five fill the phone platter");
        for(size_t selected=0;selected<count;++selected) {
            tabs->setSelectedIndex(selected);tabs->selectedViewController()->view()->setBackgroundColor(UIColor::systemGroupedBackground);settle();frame();
            const auto lens=state._lens->frame();
            NXFloat nativeWidth=count<4?94:(count==4?platterWidth/4+5:std::max(79.333333f,platterWidth/5+5));
            if(count==4&&width==320)nativeWidth=selected==1?85.349544f:81.550152f;
            if(count==4&&width==360)nativeWidth=85.333333f;
            if(count==5&&width==320)nativeWidth=selected==1?72.666667f:69.333333f;
            if(count==5&&width==360)nativeWidth=selected==1?80.829268f:77.292683f;
            if(count==5&&width==390)nativeWidth=selected==1?87.121951f:83.219512f;
            if(std::abs(lens.width()-nativeWidth)>.34f)std::cerr<<"variant "<<width<<'/'<<count<<'/'<<selected<<" lens "<<lens.width()<<" expected "<<nativeWidth<<'\n';
            expect(std::abs(lens.width()-nativeWidth)<.34f,"tab selection width matches the native count and compression regime");
            expect(std::abs(lens.midX()-state._controls[selected]->frame().midX())<.01f,
                   "selection and hit cell centers remain aligned for every tab");
            if(selected==0)expect(std::abs(lens.minX()-4)<.01f,"first selection has a four-point leading inset");
            if(selected+1==count)expect(std::abs(platterWidth-lens.maxX()-4)<.01f,"last selection has a four-point trailing inset");
            if(report){if(comma)report<<',';comma=true;report<<"{\"width\":"<<width<<",\"count\":"<<count<<",\"selected\":"<<selected<<",\"dark\":"<<(dark?"true":"false")<<",\"platter\":";rect(report,bar->frame());report<<",\"lens\":";rect(report,{bar->frame().origin+lens.origin,lens.size});report<<",\"childBottom\":"<<tabs->selectedViewController()->view()->safeAreaInsets().bottom<<'}';}
            if(!images.empty()&&(selected==0||selected+1==count)) {
                const auto name=std::to_string(int(width))+"-"+std::to_string(count)+"-"+(selected==0?"first":"last")+"-"+(dark?"dark":"light");
                auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(int(width*2),1748));surface->getCanvas()->scale(2,2);surface->getCanvas()->clear(dark?SK_ColorBLACK:SkColorSetRGB(242,242,247));
                UITraitCollection::setCurrent(window->traitCollection());window->layer()->skiaRender(surface->getCanvas());
                SkBitmap pixels;pixels.allocN32Pixels(int(width*2),1748);surface->readPixels(pixels,0,0);
                std::ofstream out(images+"/"+name+".ppm",std::ios::binary);out<<"P6\n"<<pixels.width()<<' '<<pixels.height()<<"\n255\n";
                for(int y=0;y<pixels.height();++y)for(int x=0;x<pixels.width();++x){const auto color=pixels.getColor(x,y);const unsigned char rgb[]={uint8_t(SkColorGetR(color)),uint8_t(SkColorGetG(color)),uint8_t(SkColorGetB(color))};out.write(reinterpret_cast<const char*>(rgb),3);}
            }
        }
        tabs->setSelectedIndex(0);settle();frame();
        const auto lastControl=state._controls.back();
        const auto last=lastControl->convertToView({lastControl->bounds().width()*.5f,31},window);
        auto touch=new_shared<UITouch>(10000+count,last,Timer());auto event=UIGestureRecognizerTestHarness::event(window,touch);
        UIGestureRecognizerTestHarness::send(window,event,touch,UITouchPhase::began,last,0);
        UIGestureRecognizerTestHarness::send(window,event,touch,UITouchPhase::ended,last,32);
        settleTabLift(bar);
        expect(tabs->selectedIndex()==count-1&&std::abs(state._lens->frame().maxX()-(platterWidth-4))<.01f,
               "routed last-tab taps preserve selection and the trailing lens inset for every count");
        const auto firstControl=state._controls.front();
        const auto first=firstControl->convertToView({firstControl->bounds().width()*.5f,31},window);
        auto drag=new_shared<UITouch>(11000+count,last,Timer());auto dragEvent=UIGestureRecognizerTestHarness::event(window,drag);
        UIGestureRecognizerTestHarness::send(window,dragEvent,drag,UITouchPhase::began,last,0);
        UIGestureRecognizerTestHarness::send(window,dragEvent,drag,UITouchPhase::moved,(last+first)*.5f,16);
        UIGestureRecognizerTestHarness::send(window,dragEvent,drag,UITouchPhase::moved,first,32);
        UIGestureRecognizerTestHarness::send(window,dragEvent,drag,UITouchPhase::ended,first,48);
        settleTabLift(bar);
        expect(tabs->selectedIndex()==0&&std::abs(state._lens->frame().minX()-4)<.01f,
               "routed drags return to the first tab and restore its leading lens inset");
        window->setRootViewController(nullptr);UIApplication::shared->delegate->window.reset();
    }
    if(report)report<<"]}\n";
}
}
int main(int argc,char** argv) {
    auto ctx=std::make_shared<Context>();SkiaCtx::_main=ctx;
    UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
    barCases(ctx);controllerCases(ctx);settle();
    if(argc>2)std::filesystem::create_directories(argv[2]);
    variantCases(ctx,argc>1?argv[1]:"",argc>2?argv[2]:"");
    UIApplication::shared.reset();SkiaCtx::_main.reset();
    if(!failures)std::cout<<"UITabBarView and UITabBarController regressions passed\n";
    return failures?1:0;
}
