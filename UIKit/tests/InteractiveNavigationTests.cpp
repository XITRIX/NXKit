#include <UIKit.h>
#ifdef NXKIT_TEST_NX_NAVIGATION
#include <NXNavigationController.h>
#endif
#include <UIApplication.h>
#include <UIWindow.h>
#include <UITouch.h>
#include <UIEvent.h>
#include <UIPanGestureRecognizer.h>
#include <SkiaCtx.h>
#include "UINavigationBarInternal.hpp"
#include "LayerAnimationTimeline.hpp"
#include "platforms/apple/AppleSystemFont.hpp"
#include "include/ports/SkFontMgr_mac_ct.h"
#include "include/core/SkSurface.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <filesystem>
using namespace NXKit;
namespace NXKit {
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>){return true;}
class DispatchQueueTestHarness {public:static void pump(){DispatchQueue::main()->performAll();}};
class UIGestureRecognizerTestHarness {
public:
    std::shared_ptr<UIWindow> window;std::shared_ptr<UITouch> touch;std::shared_ptr<UIEvent> event;Timer origin;
    explicit UIGestureRecognizerTestHarness(std::shared_ptr<UIWindow> w,NXKit::NXPoint p={2,300}):window(std::move(w)) {
        static unsigned long next=92;touch=new_shared<UITouch>(++next,p,origin);event=new_shared<UIEvent>();event->_allTouches.insert(touch);touch->_window=window;window->sendEvent(event);
    }
    void send(UITouchPhase phase,NXKit::NXPoint p,double milliseconds) {
        touch->updateAbsoluteLocation(p);touch->_timestamp=origin;const auto us=origin.startTime.tv_usec+long(milliseconds*1000);touch->_timestamp.startTime.tv_sec+=us/1000000;touch->_timestamp.startTime.tv_usec=us%1000000;touch->_phase=phase;window->sendEvent(event);
    }
};
}
namespace {
int failures=0;
void expect(bool condition,const char* message){if(!condition){++failures;std::cerr<<"FAIL: "<<message<<'\n';}}
bool close(float a,float b){return std::abs(a-b)<.01f;}
class Context:public SkiaCtx {
public:
    NXKit::NXSize size{402,874};bool dark=false;float density=3;
    Context(){fontMgr=SkFontMgr_New_CoreText(nullptr);}
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style=SkFontStyle(),NXFloat size=17)const override{return Internal::appleSystemTypeface(style,size);}
    NXKit::NXSize getSize()override{return size;}float getScaleFactor()override{return density;}
    std::array<NXFloat,4> windowCornerRadii()override{return {62,54,46,38};}
    UIEdgeInsets deviceSafeAreaInsets()override{return {62,0,34,0};}
    UIUserInterfaceStyle getThemeMode()override{return dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;}
    sk_sp<SkSurface> getBackbufferSurface()override{return nullptr;}void swapBuffers()override{}void sleepForNextFrame()override{}bool platformRunLoop(std::function<bool()>)override{return false;}
protected:skgpu::graphite::Context* graphiteContext()override{return nullptr;}skgpu::graphite::Recorder* graphiteRecorder()override{return nullptr;}
};
std::vector<std::string> events;
class Screen:public UIViewController {
public:
    std::function<void()> onDisappear,onAppear;
    std::function<void(const std::shared_ptr<UIView>&)> onLoad;
    explicit Screen(std::string title){setTitle(std::move(title));}
    void loadView()override{auto root=new_shared<UIView>();setView(root);if(onLoad)onLoad(root);}
    void viewWillAppear(bool)override{events.push_back(title()+".willAppear");if(const auto callback=onAppear)callback();}
    void viewDidAppear(bool)override{events.push_back(title()+".didAppear");}
    void viewWillDisappear(bool)override{events.push_back(title()+".willDisappear");if(const auto callback=onDisappear)callback();}
    void viewDidDisappear(bool)override{events.push_back(title()+".didDisappear");}
};
void settle(){for(int i=0;i<6;++i){UIView::completePendingAnimations();DispatchQueueTestHarness::pump();}}
float x(const std::shared_ptr<UIViewController>& screen) {const auto host=screen->view()->superview().lock();return host?NXTransform3DGetAffineTransform(host->layer()->presentationOrSelf()->transform()).tX:0;}
struct Fixture {
    std::shared_ptr<Screen> root=new_shared<Screen>("Root"),target=new_shared<Screen>("Target"),source=new_shared<Screen>("Source");
    std::shared_ptr<UINavigationController> nav=new_shared<UINavigationController>(root);
    std::shared_ptr<UIWindow> window=new_shared<UIWindow>();
    Fixture(){nav->setViewControllers({root,target,source},false);UIApplication::shared->delegate->window=window;window->setRootViewController(nav);window->makeKeyAndVisible();window->drawAndLayoutTreeIfNeeded();events.clear();}
    ~Fixture(){settle();window->setRootViewController(nullptr);UIApplication::shared->delegate->window.reset();UIApplication::shared->keyWindow.reset();}
};
void tracking(bool commit,bool cancellation=false) {
    Fixture f;f.source->onAppear=[&]{expect(f.nav->topViewController()==f.source&&f.nav->viewControllers().size()==3,"cancelled appearance callbacks observe the restored logical stack");};auto recognizer=f.nav->interactivePopGestureRecognizer();expect(recognizer&&recognizer->isEnabled()&&recognizer->view().lock()==f.nav->view(),"UIKit installs an enabled gesture on its container");
    expect(f.source->navigationController()->interactivePopGestureRecognizer()==recognizer,"shared navigation discovery exposes the same gesture");
    auto& bar=Internal::NavigationBarAccess::state(*f.nav->navigationBar());
    const auto back=bar.left.buttons.front();const auto glass=bar.left.groups.front().view;
    const auto identifier=back->item->identifier();const auto backBounds=glass->layer()->bounds();const auto backPosition=glass->layer()->position();
    auto stableBack=[&]{const auto pose=glass->layer()->presentationOrSelf();expect(identifier&&back->item->identifier()==identifier&&bar.left.buttons.front()==back&&bar.left.groups.front().view==glass,"interactive pop preserves the unchanged automatic Back identity");expect(pose->bounds()==backBounds&&pose->position()==backPosition&&pose->transform()==NXTransform3D::identity,"unchanged Back stays stationary through interactive tracking and settlement");};
    int observed=0;recognizer->onStateChanged=[&](const auto&){++observed;};
    UIGestureRecognizerTestHarness input(f.window);input.send(UITouchPhase::moved,{14,300},50);input.send(UITouchPhase::moved,{114,300},250);
    expect(f.nav->viewControllers().size()==2&&f.nav->topViewController()==f.target&&!f.source->parent().expired(),"interactive pop changes logical top while retaining the outgoing child");
    expect(close(x(f.source),100)&&close(x(f.target),-90.6f),"content tracks finger distance with destination parallax");
    for(const auto& screen:{f.source,f.target}) {
        const auto host=screen->view()->superview().lock();
        expect(host->clipsToBounds()&&close(host->effectiveRadius(UIRectCorner::topLeft),62)&&close(host->effectiveRadius(UIRectCorner::bottomRight),38),"interactive navigation cards retain independent platform corners while moving");
        expect(screen->view()->effectiveRadius(UIRectCorner::allCorners)==0&&!screen->view()->clipsToBounds(),"card wrappers preserve controller-owned corner styling");
    }
    stableBack();
    const auto before=x(f.source);UIView::completePendingAnimations();expect(close(x(f.source),before)&&f.nav->viewControllers().size()==2,"holding the gesture cannot complete its animations");
    input.send(UITouchPhase::moved,{214,300},450);input.send(UITouchPhase::moved,{114,300},650);expect(close(x(f.source),100),"reversing finger travel reverses presentation geometry");
    stableBack();
    if(commit)input.send(UITouchPhase::moved,{314,300},850);
    input.send(UITouchPhase::moved,{commit?314.f:114.f,300},1000);input.send(UITouchPhase::moved,{commit?314.f:114.f,300},1200);
    if(cancellation)recognizer->setEnabled(false);else input.send(UITouchPhase::ended,{commit?314.f:114.f,300},1250);
    if(!commit){expect(f.nav->viewControllers().size()==2,"cancelled UIKit pop restores the logical stack at completion");expect(events==std::vector<std::string>{"Source.willDisappear","Target.willAppear"},"cancelled appearance reconciliation waits for the visual return to finish");}
    settle();
    stableBack();
    expect(observed>=4,"application observation preserves the framework's internal gesture action");
    expect(f.nav->topViewController()==(commit?f.target:f.source)&&f.nav->viewControllers().size()==(commit?2:3),"commit/cancel chooses the correct final stack");
    expect(f.nav->visibleViewController()==f.nav->topViewController()&&f.nav->navigationBar()->topItem()==f.nav->topViewController()->navigationItem(),"visible controller and bar synchronize after settlement");
    expect(close(x(f.nav->topViewController()),0)&&!f.nav->topViewController()->view()->superview().lock()->layer()->presentation(),"settlement leaves no content transform or presentation behind");
    expect(!f.nav->topViewController()->view()->superview().lock()->clipsToBounds()&&f.nav->topViewController()->view()->superview().lock()->effectiveRadius(UIRectCorner::allCorners)==0,"commit/cancel clears temporary card clipping");
    expect(commit?f.source->parent().expired():f.source->parent().lock()==f.nav,"cancel retains containment, commit detaches the outgoing controller");
    const std::vector<std::string> cancelled={"Source.willDisappear","Target.willAppear","Target.willDisappear","Target.didDisappear","Source.willAppear","Source.didAppear"};
    const std::vector<std::string> completed={"Source.willDisappear","Target.willAppear","Source.didDisappear","Target.didAppear"};
    expect(events==(commit?completed:cancelled),"appearance callback order agrees with observed UIKit completion/cancellation");
    // Reusing the very same recognizer after a cancellation must work.
    input.send(UITouchPhase::ended,{114,300},1400);recognizer->setEnabled(true);recognizer->onStateChanged={};
}
void titleSettlement(const std::shared_ptr<Context>& context) {
    struct Release {float progress,velocity;bool commit;};
    std::ofstream trace;if(const auto path=std::getenv("NXKIT_NAVIGATION_TITLE_TRACE")){trace.open(path);trace<<"dark,scale,custom_titles,progress,velocity,frame,source_content,source_title,target_content,target_title,reveal_width\n";}
    for(bool dark:{false,true})for(float density:{1.f,2.f,3.f})for(int custom:{0,1,2,3})for(auto release:{Release{.25f,0,false},Release{.55f,0,true},Release{.8f,0,true},Release{.4f,600,true},Release{.6f,-200,false}}) {
        const auto [progress,velocity,commit]=release;
        context->dark=dark;context->density=density;
        Fixture f;settle();
        const auto sourceCustom=custom&1?new_shared<UIView>(NXRect{0,0,80,30}):nullptr;
        const auto targetCustom=custom&2?new_shared<UIView>(NXRect{0,0,100,32}):nullptr;
        const auto applicationTransform=NXAffineTransform::scale(.9f);
        for(const auto& title:{sourceCustom,targetCustom})if(title)title->setTransform(applicationTransform);
        if(sourceCustom)f.source->navigationItem()->setTitleView(sourceCustom);
        if(targetCustom)f.target->navigationItem()->setTitleView(targetCustom);
        auto& bar=Internal::NavigationBarAccess::state(*f.nav->navigationBar());
        const auto outgoing=bar.titleInk;
        UIGestureRecognizerTestHarness input(f.window);
        input.send(UITouchPhase::moved,{14,300},50);
        const auto finger=14+402*progress;
        input.send(UITouchPhase::moved,{finger,300},250);
        const auto incoming=bar.interactiveTitle;
        auto translation=[](const auto& view){return NXTransform3DGetAffineTransform(view->layer()->presentationOrSelf()->transform()).tX;};
        expect(close(translation(outgoing),x(f.source))&&close(translation(incoming),x(f.target)),"both titles track their own content while held");
        // Cover a paused release, a forward flick below half, and a backwards
        // release above half. The same title/content relation must hold for all.
        input.send(UITouchPhase::moved,{finger,300},600);
        if(velocity){input.send(UITouchPhase::moved,{finger-velocity*.1f,300},700);input.send(UITouchPhase::moved,{finger-velocity*.05f,300},750);}
        input.send(UITouchPhase::moved,{finger,300},800);
        input.send(UITouchPhase::ended,{finger,300},850);
        expect((commit?bar.titleInk:bar.interactiveTitle)==incoming,"releasing a pop preserves the incoming title carrier");
        const auto origin=Timer();
        // Include the natural completion boundary and the material retirement
        // tail. Forcing all animations to finish at frame 59 hides a title
        // returning to its model position between these two clocks.
        for(int frame=0;frame<=144;++frame) {
            auto now=origin;const auto us=origin.startTime.tv_usec+frame*1000000L/120;
            now.startTime.tv_sec+=us/1000000;now.startTime.tv_usec=us%1000000;
            std::function<void(const std::shared_ptr<CALayer>&)> tick=[&](const auto& layer){const auto children=layer->sublayers();layer->animateAt(now);for(const auto& child:children)tick(child);};
            tick(f.window->layer());
            DispatchQueueTestHarness::pump();
            if(frame>=60) {
                expect(f.nav->topViewController()==(commit?f.target:f.source),"natural completion selects the intended controller before the material tail ends");
                expect(commit?outgoing->superview().expired():incoming->superview().expired(),"natural completion detaches the obsolete title before its animation can reveal its model pose");
                expect(bar.titleInk==(commit?incoming:outgoing)&&bar.titleInk->superview().lock()==f.nav->navigationBar(),"exactly the destination title carrier remains attached after natural completion");
                expect(close(translation(bar.titleInk),0),"the settled title remains stationary through the material cleanup tail");
                if(const auto destination=std::getenv("NXKIT_NAVIGATION_POP_COMPLETION_PICTURES");destination&&custom==0&&density==3&&commit&&progress==.55f&&(frame==60||frame==72||frame==96||frame==120)) {
                    f.window->drawAndLayoutTreeIfNeeded();
                    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(402*3,54*3));auto canvas=surface->getCanvas();
                    UITraitCollection::setCurrent(f.window->traitCollection());canvas->clear(UIColor::systemBackground.raw());canvas->scale(3,3);canvas->translate(0,-f.nav->navigationBar()->frame().minY());f.nav->navigationBar()->layer()->skiaRender(canvas);
                    const std::filesystem::path directory=destination;std::filesystem::create_directories(directory);
                    SkFILEWStream file((directory/((dark?"dark-":"light-")+std::to_string(frame)+".png")).c_str());SkBitmap image;image.allocN32Pixels(402*3,54*3);surface->readPixels(image,0,0);
                    expect(SkPngEncoder::Encode(&file,image.pixmap(),{}),"post-gesture title comparison encodes successfully");
                }
                continue;
            }
            expect(std::abs(translation(outgoing)-x(f.source))<.02f,"outgoing title stays attached to its screen on every release frame");
            expect(std::abs(translation(incoming)-x(f.target))<.02f,"incoming title keeps destination parallax throughout settlement");
            const auto expectedWidth=402*(.3f+.7f*x(f.source)/402);
            expect(std::abs(incoming->layer()->presentationOrSelf()->bounds().width()-expectedWidth)<.02f,"title reveal boundary shares the content settling clock");
            if(sourceCustom)expect(sourceCustom->superview().lock()==outgoing&&sourceCustom->transform()==applicationTransform,"custom outgoing title travels once with its carrier and preserves its transform");
            if(targetCustom)expect(targetCustom->superview().lock()==incoming&&targetCustom->transform()==applicationTransform,"custom incoming title remains on its destination carrier until settlement");
            if(trace)trace<<dark<<','<<density<<','<<custom<<','<<progress<<','<<velocity<<','<<frame<<','<<x(f.source)<<','<<translation(outgoing)<<','<<x(f.target)<<','<<translation(incoming)<<','<<incoming->layer()->presentationOrSelf()->bounds().width()<<'\n';
        }
        settle();
        expect(f.nav->topViewController()==(commit?f.target:f.source),"title settlement keeps the intended commit/cancel outcome");
        const auto visibleCustom=commit?targetCustom:sourceCustom;
        if(visibleCustom)expect(visibleCustom->superview().lock()==f.nav->navigationBar()&&visibleCustom->transform()==applicationTransform,"settled custom title restores the ordinary bar hierarchy and application transform");
    }
    context->dark=false;context->density=3;
}
class Veto:public UIGestureRecognizerDelegate {bool gestureRecognizerShouldBegin(std::shared_ptr<UIGestureRecognizer>)override{return false;}};
class DisableOnBegin:public UIGestureRecognizerDelegate {public:bool reenable=false;bool gestureRecognizerShouldBegin(std::shared_ptr<UIGestureRecognizer> gesture)override{gesture->setEnabled(false);if(reenable)gesture->setEnabled(true);return true;}};
void admission() {
    for(int variant=0;variant<7;++variant){Fixture f;auto gesture=f.nav->interactivePopGestureRecognizer();auto veto=std::make_shared<Veto>();auto disable=std::make_shared<DisableOnBegin>();
        if(variant==1)gesture->setEnabled(false);if(variant==2)gesture->delegate=veto;
        if(variant>=5){disable->reenable=variant==6;gesture->delegate=disable;}
        if(variant==3)f.source->navigationItem()->setHidesBackButton(true);
        if(variant==4)f.nav->setViewControllers({f.root},false);
        UIGestureRecognizerTestHarness input(f.window,variant==0?NXKit::NXPoint{100,300}:NXKit::NXPoint{2,300});input.send(UITouchPhase::moved,{220,300},100);input.send(UITouchPhase::ended,{220,300},200);settle();
        expect(f.nav->viewControllers().size()==(variant==4?1:3),"center touches, disabled/vetoed gestures, hidden Back and root cannot pop");
    }
    Fixture f;UIGestureRecognizerTestHarness input(f.window);input.send(UITouchPhase::moved,{4,340},100);input.send(UITouchPhase::ended,{4,340},200);expect(f.nav->topViewController()==f.source,"vertical edge scrolling cannot pop");
#ifdef NXKIT_TEST_NX_NAVIGATION
    auto nx=new_shared<NXNavigationController>(new_shared<UIViewController>());std::shared_ptr<UINavigationControllerInterface> shared=nx;expect(!shared->interactivePopGestureRecognizer(),"NXKit has no pop gesture installed by default");
#endif
}
void preview() {
    Fixture f;
    auto item=[](const char* symbol,const char* id,UIBarButtonItemStyle style){auto value=std::make_shared<UIBarButtonItem>(UIImage::systemImageNamed(symbol,20,400,3),style);value->setIdentifier(id);return value;};
    auto add=item("plus","add",UIBarButtonItemStyle::plain),more=item("ellipsis","more",UIBarButtonItemStyle::plain);
    auto prominent=item("plus","add",UIBarButtonItemStyle::prominent),oldMore=item("ellipsis","more",UIBarButtonItemStyle::plain);
    f.target->navigationItem()->setRightBarButtonItems({add,more});f.source->navigationItem()->setRightBarButtonItems({prominent,oldMore});
    auto bar=f.nav->navigationBar();auto& state=Internal::NavigationBarAccess::state(*bar);const auto blue=state.right.groups.front().view;
    expect(state.right.groups.size()==2&&std::dynamic_pointer_cast<BackdropEffectView>(blue)->glassEffect()->tintColor().has_value(),"source starts with separate prominent glass");
    UIGestureRecognizerTestHarness input(f.window);input.send(UITouchPhase::moved,{14,300},50);input.send(UITouchPhase::moved,{115,300},250);
    expect(state.right.groups.size()==2&&state.right.buttons.front()->item==prominent&&bar->topItem()==f.target->navigationItem(),"drag preserves outgoing styling while bar metadata follows the destination");
    expect(close(blue->layer()->presentationOrSelf()->bounds().width(),44+26*101/402.f),"drag previews half the eventual glass extent, as measured in UIKit");
    input.send(UITouchPhase::cancelled,{115,300},600);settle();
    expect(state.right.groups.size()==2&&close(blue->frame().width(),44)&&!blue->layer()->presentation(),"cancellation restores the exact original glass without retiring it");
    // UIKit identifiers are optional. New anonymous items use visual order;
    // actual reused objects keep their identity when measured through copies.
    prominent->setIdentifier({});oldMore->setIdentifier({});add->setIdentifier({});more->setIdentifier({});
    UIGestureRecognizerTestHarness again(f.window);again.send(UITouchPhase::moved,{14,300},50);again.send(UITouchPhase::moved,{115,300},250);
    expect(blue->layer()->presentationOrSelf()->bounds().width()>44,"anonymous items still participate in interactive glass grouping");
    again.send(UITouchPhase::cancelled,{115,300},600);settle();
}
void independentApplicationAnimation() {
    Fixture f;f.target->onLoad=[](const auto& root){auto animation=new_shared<CABasicAnimation>("opacity");animation->fromValue=.2f;animation->toValue=1.f;animation->duration=2;animation->animationGroup=new_shared<UIViewAnimationGroup>(UIViewAnimationOptions(allowUserInteraction),std::nullopt);root->layer()->add(animation,"applicationFade");};
    UIGestureRecognizerTestHarness input(f.window);input.send(UITouchPhase::moved,{14,300},50);input.send(UITouchPhase::moved,{114,300},250);UIView::completePendingAnimations();
    expect(!f.target->view()->layer()->presentation()&&close(x(f.source),100),"application animations started in loadView keep ordinary time while navigation is held");
    input.send(UITouchPhase::cancelled,{114,300},600);settle();
}
void rootBoundary() {
    for(bool commit:{false,true}){Fixture f;f.nav->setViewControllers({f.root,f.source},false);events.clear();const auto applicationTransform=NXAffineTransform::scale(.97f);f.source->view()->setTransform(applicationTransform);
        UIGestureRecognizerTestHarness input(f.window);input.send(UITouchPhase::moved,{14,300},50);input.send(UITouchPhase::moved,{commit?314.f:114.f,300},250);input.send(UITouchPhase::ended,{commit?314.f:114.f,300},600);settle();
        expect(f.nav->viewControllers().size()==(commit?1:2)&&f.nav->topViewController()==(commit?f.root:f.source),"root-boundary pop commits or cancels without losing the root");
        expect(f.source->view()->transform()==applicationTransform,"interactive navigation preserves application content transforms");
        if(commit)expect(Internal::NavigationBarAccess::state(*f.nav->navigationBar()).left.groups.empty(),"committing to an ordinary root retires the automatic Back glass");
    }
    {Fixture f;auto custom=new_shared<UIView>(NXRect{0,0,70,32});const auto transform=NXAffineTransform::scale(.9f);custom->setTransform(transform);f.source->navigationItem()->setTitleView(custom);f.target->navigationItem()->setTitleView(custom);
        auto button=new_shared<UIView>(NXRect{0,0,40,30});f.source->navigationItem()->setRightBarButtonItem(std::make_shared<UIBarButtonItem>(button));f.target->navigationItem()->setRightBarButtonItem(std::make_shared<UIBarButtonItem>(button));
        const auto parent=button->superview().lock();UIGestureRecognizerTestHarness input(f.window);input.send(UITouchPhase::moved,{14,300},50);input.send(UITouchPhase::moved,{114,300},250);
        expect(button->superview().lock()==parent&&custom->transform()==transform,"measuring destination chrome does not reparent custom controls or overwrite their transforms");
        input.send(UITouchPhase::cancelled,{114,300},500);settle();expect(custom->superview().lock()==f.nav->navigationBar()&&custom->transform()==transform,"cancel restores a reused custom title view");
    }
}
void interruptions() {
    {Fixture f;UIGestureRecognizerTestHarness first(f.window);first.send(UITouchPhase::moved,{14,300},50);first.send(UITouchPhase::moved,{114,300},250);
        int actions=0;auto control=new_shared<UIControl>();control->setFrame({0,0,402,874});control->primaryAction=UIAction("Unexpected",[&]{++actions;});f.source->view()->addSubview(control);
        UIGestureRecognizerTestHarness second(f.window,{3,320});settle();first.send(UITouchPhase::ended,{114,300},400);second.send(UITouchPhase::ended,{3,320},400);
        expect(f.nav->topViewController()==f.source&&f.nav->viewControllers().size()==3,"a second finger cancels safely rather than leaving a failed gesture's transition frozen");expect(actions==0&&second.touch->hasBeenCancelledByAGestureRecognizer(),"a touch that cancels an already active gesture cannot become a delayed content tap");}
    {Fixture f;f.source->onDisappear=[gesture=f.nav->interactivePopGestureRecognizer()]{gesture->setEnabled(false);};UIGestureRecognizerTestHarness input(f.window);input.send(UITouchPhase::moved,{14,300},50);input.send(UITouchPhase::ended,{14,300},100);settle();expect(f.nav->topViewController()==f.source,"disabling during willDisappear cancels after balanced initial appearance delivery");expect(std::find(events.begin(),events.end(),"Target.willAppear")<std::find(events.begin(),events.end(),"Target.willDisappear"),"cancellation requested reentrantly never disappears a target before it appears");f.source->onDisappear={};}

    for(bool resize:{false,true}) {Fixture f;UIGestureRecognizerTestHarness input(f.window);input.send(UITouchPhase::moved,{14,300},50);input.send(UITouchPhase::moved,{114,300},250);
        if(resize){f.nav->view()->setFrame({0,0,600,874});f.nav->viewDidLayoutSubviews();}else f.nav->setNavigationBarHidden(true);
        expect(f.nav->topViewController()==f.source&&f.nav->viewControllers().size()==3&&close(x(f.source),0),"resize and bar visibility changes safely cancel tracking");input.send(UITouchPhase::ended,{114,300},400);settle();}
    Fixture f;UIGestureRecognizerTestHarness input(f.window);input.send(UITouchPhase::moved,{14,300},50);input.send(UITouchPhase::moved,{314,300},150);auto later=new_shared<Screen>("Queued");f.nav->pushViewController(later,false);input.send(UITouchPhase::ended,{314,300},160);settle();expect(f.nav->topViewController()==later&&later->parent().lock()==f.nav,"stack changes requested during tracking drain only after settlement");
}
}
int main(){auto ctx=std::make_shared<Context>();SkiaCtx::_main=ctx;UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
    tracking(false);tracking(true);tracking(false,true);titleSettlement(ctx);admission();preview();independentApplicationAnimation();rootBoundary();interruptions();
    UIApplication::shared->keyWindow.reset();UIApplication::shared->delegate->window.reset();UIApplication::shared.reset();SkiaCtx::_main.reset();
    if(!failures)std::cout<<"Interactive navigation tests passed\n";return failures?1:0;
}
