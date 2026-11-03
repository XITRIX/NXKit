#include <UIKit.h>
#include <UIApplication.h>
#include <UIWindow.h>
#include <UIEvent.h>
#include <UITouch.h>
#include <SkiaCtx.h>
#include "UINavigationBarInternal.hpp"
#include "NavigationBarMotion.hpp"
#include "ViewControllerContainerAccess.hpp"
#ifdef __APPLE__
#include "platforms/apple/AppleSystemFont.hpp"
#endif
#include "include/core/SkSurface.h"
#include "include/core/SkBitmap.h"
#ifdef __APPLE__
#include "include/ports/SkFontMgr_mac_ct.h"
#endif
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdexcept>
using namespace NXKit;
namespace NXKit {
bool applicationRunLoop();
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>){return true;}
class UIGestureRecognizerTestHarness {
public:
static auto begin(const std::shared_ptr<UIWindow>& window,NXPoint point) {
    static unsigned long id=9100;auto touch=new_shared<UITouch>(++id,point,Timer());auto event=new_shared<UIEvent>();event->_allTouches.insert(touch);touch->_window=window;
    touch->_phase=UITouchPhase::began;window->sendEvent(event);return std::pair{touch,event};
}
static void send(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<UITouch>& touch,const std::shared_ptr<UIEvent>& event,UITouchPhase phase,NXPoint point) {
    touch->updateAbsoluteLocation(point);touch->_timestamp=Timer(80);touch->_phase=phase;window->sendEvent(event);
}
static void touch(const std::shared_ptr<UIWindow>& window) {
    auto touch=new_shared<UITouch>(9001,NXPoint{200,400},Timer());auto event=new_shared<UIEvent>();event->_allTouches.insert(touch);touch->_window=window;
    touch->_phase=UITouchPhase::began;window->sendEvent(event);touch->_phase=UITouchPhase::ended;window->sendEvent(event);
}
};
class GlassInteractionTestHarness {public:static void advance(const std::shared_ptr<BackdropEffectView>& view,double seconds){view->_advanceInteraction(seconds);}};
class DispatchQueueTestHarness {public:static void pump(){DispatchQueue::main()->performAll();}};
}
namespace {
int failures=0;std::string operation="launch";std::vector<std::pair<std::string,std::string>> events;
void log(std::string event){events.push_back({operation,std::move(event)});}
void expect(bool value,const char* message){if(!value){++failures;std::cerr<<"FAIL: "<<message<<'\n';}}
template<class F>bool rejects(F action){try{action();return false;}catch(const std::invalid_argument&){return true;}catch(const std::logic_error&){return true;}}
class Context final:public SkiaCtx {
public:
    NXKit::NXSize size{402,874};bool dark=false;float density=3;
    Context(){
#ifdef __APPLE__
        fontMgr=SkFontMgr_New_CoreText(nullptr);
#endif
    }
#ifdef __APPLE__
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style=SkFontStyle(),NXFloat size=17)const override{return Internal::appleSystemTypeface(style,size);}
#endif
    NXKit::NXSize getSize()override{return size;}
    float getScaleFactor()override{return density;}
    std::array<NXFloat,4> windowCornerRadii()override{return {62,54,46,38};}
    UIEdgeInsets deviceSafeAreaInsets()override{return {62,0,34,0};}
    UIUserInterfaceStyle getThemeMode()override{return dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;}
    sk_sp<SkSurface> getBackbufferSurface()override{return nullptr;}
    void swapBuffers()override{}void sleepForNextFrame()override{}
    bool platformRunLoop(std::function<bool()>)override{return false;}
protected:
    skgpu::graphite::Context* graphiteContext()override{return nullptr;}
    skgpu::graphite::Recorder* graphiteRecorder()override{return nullptr;}
};
class Screen final:public UIViewController {
public:
    int loads=0,appearances=0,disappearances=0;std::function<void()> onDisappear;
    explicit Screen(std::string name){setTitle(std::move(name));}
    void loadView()override{++loads;log(title()+".load");auto root=new_shared<UIView>();root->setBackgroundColor(UIColor::systemGroupedBackground);setView(root);}
    void viewWillAppear(bool)override{expect(viewIsLoaded(),"incoming view loads before appearance");log(title()+".willAppear");}
    void viewDidAppear(bool)override{++appearances;log(title()+".didAppear");}
    void viewWillDisappear(bool)override{log(title()+".willDisappear");const auto callback=onDisappear;if(callback)callback();}
    void viewDidDisappear(bool)override{++disappearances;log(title()+".didDisappear");}
};
class Delegate final:public UINavigationControllerDelegate {
    void navigationControllerWillShow(const std::shared_ptr<UINavigationController>&,const std::shared_ptr<UIViewController>& screen,bool)override{log("willShow:"+screen->title());}
    void navigationControllerDidShow(const std::shared_ptr<UINavigationController>&,const std::shared_ptr<UIViewController>& screen,bool)override{log("didShow:"+screen->title());}
};
class BarDelegate final:public UINavigationBarDelegate {
public:bool veto=false;
    bool navigationBarShouldPushItem(const std::shared_ptr<UINavigationBar>&,const std::shared_ptr<UINavigationItem>&)override{log("bar.shouldPush");return !veto;}
    void navigationBarDidPushItem(const std::shared_ptr<UINavigationBar>&,const std::shared_ptr<UINavigationItem>&)override{log("bar.didPush");}
    bool navigationBarShouldPopItem(const std::shared_ptr<UINavigationBar>&,const std::shared_ptr<UINavigationItem>&)override{log("bar.shouldPop");return !veto;}
    void navigationBarDidPopItem(const std::shared_ptr<UINavigationBar>&,const std::shared_ptr<UINavigationItem>&)override{log("bar.didPop");}
};
void frame(){for(int i=0;i<5;++i){applicationRunLoop();DispatchQueueTestHarness::pump();}}
void settle(){for(int i=0;i<12;++i){UIView::completePendingAnimations();DispatchQueueTestHarness::pump();}}
void rect(std::ostream& out,NXRect r){out<<'['<<r.minX()<<','<<r.minY()<<','<<r.width()<<','<<r.height()<<']';}
void snapshot(const std::shared_ptr<UIWindow>& window,const std::string& path,int scale) {
    window->drawAndLayoutTreeIfNeeded();auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(402*scale,874*scale));surface->getCanvas()->scale(scale,scale);surface->getCanvas()->clear(SK_ColorWHITE);window->layer()->skiaRender(surface->getCanvas());
    SkBitmap bitmap;bitmap.allocN32Pixels(402*scale,874*scale);surface->readPixels(bitmap,0,0);
    std::ofstream out(path,std::ios::binary);out<<"P6\n"<<bitmap.width()<<' '<<bitmap.height()<<"\n255\n";
    for(int y=0;y<bitmap.height();++y)for(int x=0;x<bitmap.width();++x){const auto c=bitmap.getColor(x,y);const char bytes[]={char(SkColorGetR(c)),char(SkColorGetG(c)),char(SkColorGetB(c))};out.write(bytes,3);}
}
auto groupingItems(const std::string& name) {
    auto image=[](const char* symbol,const char* id){auto item=std::make_shared<UIBarButtonItem>(UIImage::systemImageNamed(symbol,20,400,3),UIBarButtonItemStyle::plain);item->setIdentifier(id);return item;};
    auto a=image("plus","a"),b=image("ellipsis","b"),c=image("trash","c");
    if(name=="single")return std::vector{a};
    if(name=="tinted")a->setTintColor(UIColor::systemPink);
    if(name=="prominent"||name=="both-prominent")a->setStyle(UIBarButtonItemStyle::prominent);
    if(name=="both-prominent")b->setStyle(UIBarButtonItemStyle::prominent);
    if(name=="text-pair"||name=="text-image"){a=std::make_shared<UIBarButtonItem>("Edit",UIBarButtonItemStyle::plain);a->setIdentifier("a");}
    if(name=="text-pair"){b=std::make_shared<UIBarButtonItem>("Save",UIBarButtonItemStyle::plain);b->setIdentifier("b");}
    if(name.starts_with("fixed-"))return std::vector{a,UIBarButtonItem::fixedSpaceItemOfWidth(std::stof(name.substr(6))),b};
    if(name=="flexible")return std::vector{a,std::make_shared<UIBarButtonItem>(UIBarButtonSystemItem::flexibleSpace),b};
    if(name=="unshared")a->setSharesBackground(false);
    if(name=="hidden")a->setHidesSharedBackground(true);
    if(name=="disabled")a->setEnabled(false);
    if(name=="custom"){auto custom=new_shared<UIButton>();custom->setText("Menu");custom->setFrame({0,0,60,32});a=std::make_shared<UIBarButtonItem>(custom);a->setIdentifier("a");}
    if(name=="three")return std::vector{a,b,c};
    if(name=="middle-prominent"){b->setStyle(UIBarButtonItemStyle::prominent);return std::vector{a,b,c};}
    if(name=="middle-hidden"){b->setHidesSharedBackground(true);return std::vector{a,b,c};}
    if(name=="done"){a=std::make_shared<UIBarButtonItem>(UIBarButtonSystemItem::done);a->setIdentifier("a");}
    return std::vector{a,b};
}
void grouping(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<Context>& ctx,const char* reportPath,const char* pictures) {
    auto root=new_shared<UIViewController>();root->setTitle("Groups");root->view()->setBackgroundColor(UIColor::systemGroupedBackground);
    auto nav=new_shared<UINavigationController>(root);ctx->size={402,874};ctx->density=3;window->setRootViewController(nav);window->makeKeyAndVisible();frame();
    UIGestureRecognizerTestHarness::touch(window);settle();
    auto bar=nav->navigationBar();auto& state=Internal::NavigationBarAccess::state(*bar);
    std::ofstream out;if(reportPath)out.open(reportPath);if(out)out<<"{\"states\":[";bool comma=false;
    auto capture=[&](const std::string& name){
        if(!out)return;if(comma)out<<',';comma=true;out<<"{\"name\":\""<<name<<"\",\"groups\":[";bool delimiter=false;
        for(const auto* side:{&state.left,&state.right})for(const auto& group:side->groups)if(!group.view->isHidden()) {
            if(delimiter)out<<',';delimiter=true;out<<"{\"frame\":";rect(out,group.view->frame());out<<",\"material\":"<<(!group.hiddenBackground?"true":"false")<<",\"items\":[";bool d=false;
            for(const auto& entry:group.buttons){if(d)out<<',';d=true;out<<'"'<<entry->item->identifier().value_or("back")<<'"';}out<<"]}";
        }
        out<<"],\"title\":";rect(out,state.title->frame());out<<"}";
    };
    const std::vector<std::string> names={"single","pair","tinted","prominent","both-prominent","text-pair","text-image","fixed-0","fixed-12","fixed-30","flexible","unshared","hidden","disabled","custom","three","middle-prominent","middle-hidden","done"};
    for(bool left:{false,true})for(bool dark:{false,true})for(const auto& name:names) {
        ctx->dark=dark;frame();auto items=groupingItems(name);root->navigationItem()->setLeftBarButtonItems(left?items:std::vector<std::shared_ptr<UIBarButtonItem>>{});root->navigationItem()->setRightBarButtonItems(left?std::vector<std::shared_ptr<UIBarButtonItem>>{}:items);frame();
        auto& side=left?state.left:state.right;const auto label=std::string(left?"left-":"right-")+(dark?"dark-":"light-")+name;
        size_t expected=1;
        if(name=="prominent"||name=="both-prominent"||name=="text-pair"||name=="text-image"||name.starts_with("fixed-")||name=="flexible"||name=="unshared"||name=="hidden"||name=="done")expected=2;
        if(name=="middle-prominent"||name=="middle-hidden")expected=3;
        expect(side.groups.size()==expected,"native image/text/prominent/space grouping rules");
        expect(side.backgrounds.size()==expected-(name=="hidden"||name=="middle-hidden"?1:0),"hidesSharedBackground removes only that item's glass material");
        for(const auto& group:side.groups)for(const auto& entry:group.buttons)expect(entry->control->superview().lock()==group.view,"item content is owned by its actual glass capsule");
        settle();frame();capture(label);if(pictures&&!left)snapshot(window,std::string(pictures)+"/"+label+".ppm",3);
        const auto buttons=side.buttons;
        for(const auto& entry:buttons)if(!entry->spacer&&entry->item->isEnabled()) {
            int actions=0;const auto previous=entry->item->primaryAction();entry->item->setPrimaryAction(UIAction(entry->item->title(),[&]{++actions;}));frame();
            if(const auto custom=std::dynamic_pointer_cast<UIControl>(entry->custom))custom->primaryAction=UIAction("Custom",[&]{++actions;});
            const auto point=entry->control->convertToView({entry->width*.5f,22},window);auto [touch,event]=UIGestureRecognizerTestHarness::begin(window,point);
            UIGestureRecognizerTestHarness::send(window,touch,event,UITouchPhase::moved,point+NXKit::NXPoint{-6,4});
            size_t moving=0;for(const auto& glass:side.backgrounds){GlassInteractionTestHarness::advance(glass,.15);moving+=!glass->interactionTransform().isIdentity();}
            expect(moving==(entry->item->hidesSharedBackground()?0:1),"each enabled item moves exactly its own material group");
            UIGestureRecognizerTestHarness::send(window,touch,event,UITouchPhase::ended,point);expect(actions==1,"group interaction delivers exactly one action");
            for(const auto& glass:side.backgrounds)GlassInteractionTestHarness::advance(glass,2);
            entry->item->setPrimaryAction(previous);
            if(const auto custom=std::dynamic_pointer_cast<UIControl>(entry->custom))custom->primaryAction.reset();
        }
    }
    root->navigationItem()->setLeftBarButtonItems({});root->navigationItem()->setRightBarButtonItems(groupingItems("pair"));frame();
    const std::vector<std::string> transitions={"single","pair","prominent","fixed-0","pair","three","middle-prominent","pair","text-pair","pair"};
    if(out)out<<"],\"motions\":[";comma=false;
    for(const auto& name:transitions)for(bool pop:{false,true}) {
        if(!pop){auto child=new_shared<UIViewController>();child->setTitle("Detail");child->navigationItem()->setRightBarButtonItems(groupingItems(name));nav->pushViewController(child,true);}else nav->popViewController(true);
        expect(!state.right.groups.empty(),"push/pop retains live glass carriers");
        for(const auto& group:state.right.groups)for(const auto& entry:group.buttons)expect(entry->control->superview().lock()==group.view,"transitioning icon and glass share a hierarchy");
        if(out){if(comma)out<<',';comma=true;out<<"{\"operation\":\""<<(pop?"pop-":"push-")<<name<<"\",\"frames\":[";}
        const auto origin=Timer();
        for(int sample=0;sample<=120;++sample) {
            auto now=origin;const long us=now.startTime.tv_usec+sample*1000000/120;now.startTime.tv_sec+=us/1000000;now.startTime.tv_usec=us%1000000;
            if(out){if(sample)out<<',';out<<"{\"t\":"<<sample/120.0<<",\"groups\":[";}
            bool d=false;
            for(const auto* side:{&state.left,&state.right})for(const auto& group:side->groups) {
                group.view->layer()->animateAt(now);const auto p=group.view->layer()->presentationOrSelf();const auto size=p->bounds().size;const auto pos=p->position();
                if(out&&p->opacity()>.001f){if(d)out<<',';d=true;auto shown=NXRect{-size.width*.5f,-size.height*.5f,size.width,size.height}.applying(p->transform());shown.origin+=pos;rect(out,shown);}
                for(const auto& entry:group.buttons){entry->control->layer()->animateAt(now);const auto ink=entry->control->layer()->presentationOrSelf();expect(ink->position().x>=-1&&ink->position().x<=size.width+1,"morphing item center remains inside its own glass capsule");}
            }
            if(out)out<<"]}";
        }
        if(out)out<<"]}";
        settle();frame();
        for(const auto& group:state.right.groups)expect(!group.view->layer()->presentation(),"group morphs finish without orphaned presentation layers");
    }
    // Retarget an in-flight standalone split into a join, and reorder the same
    // identifiers. No retiring completion may remove a newly active control.
    auto standalone=new_shared<UINavigationBar>(NXRect{0,0,402,54});root->view()->addSubview(standalone);auto item=std::make_shared<UINavigationItem>();item->setRightBarButtonItems(groupingItems("pair"));standalone->setItems({item});
    auto& live=Internal::NavigationBarAccess::state(*standalone);item->setRightBarButtonItems(groupingItems("fixed-0"),true);
    for(const auto& group:live.right.groups)group.view->layer()->animateAt(Timer(150));
    const auto kept=live.right.groups.back().view;
    auto visible=[](const auto& view){const auto p=view->layer()->presentationOrSelf();const auto size=p->bounds().size;auto rect=NXRect{-size.width*.5f,-size.height*.5f,size.width,size.height}.applying(p->transform());rect.origin+=p->position();return rect;};
    const auto beforeJoin=visible(kept);item->setRightBarButtonItems(groupingItems("pair"),true);
    kept->layer()->animateAt(Timer());const auto afterJoin=visible(kept);
    expect(std::abs(beforeJoin.width()-afterJoin.width())<.1f&&std::abs(beforeJoin.minX()-afterJoin.minX())<.1f,"interrupted morph retargets from the visible capsule, including its pulse transform");
    settle();frame();
    expect(live.right.groups.size()==1&&live.right.groups.front().view->superview().lock()!=nullptr,"interrupted split/join retains its active group");
    auto ids=item->rightBarButtonItems();const auto firstControl=live.right.buttons.front()->control;std::reverse(ids.begin(),ids.end());item->setRightBarButtonItems(ids,true);settle();
    expect(live.right.buttons.back()->control==firstControl,"identifier matching preserves item presentation across reorder");
    standalone->removeFromSuperview();
    auto markerHost=new_shared<UIView>(NXRect{0,0,36,44}),marker=new_shared<UIView>(NXRect{8,14,8,8});marker->setBackgroundColor(UIColor::systemRed);markerHost->addSubview(marker);
    auto markerItem=std::make_shared<UIBarButtonItem>(markerHost);root->navigationItem()->setRightBarButtonItem(markerItem);frame();
    const auto glass=state.right.backgrounds.front();const auto control=state.right.buttons.front()->control;
    const auto contact=control->convertToView({18,22},window);auto [touch,event]=UIGestureRecognizerTestHarness::begin(window,contact);
    UIGestureRecognizerTestHarness::send(window,touch,event,UITouchPhase::moved,contact+NXKit::NXPoint{8,5});GlassInteractionTestHarness::advance(glass,.25);
    const auto local=marker->convertToView({4,4},glass);const auto expected=glass->convertToView(local.applying(glass->interactionTransform()),window);
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1206,2622));surface->getCanvas()->scale(3,3);surface->getCanvas()->clear(SK_ColorBLACK);window->drawAndLayoutTreeIfNeeded();window->layer()->skiaRender(surface->getCanvas());
    SkBitmap pixels;pixels.allocN32Pixels(1206,2622);surface->readPixels(pixels,0,0);double sumX=0,sumY=0,count=0;
    for(int y=186;y<348;++y)for(int x=900;x<1206;++x){const auto color=pixels.getColor(x,y);if(SkColorGetR(color)>200&&SkColorGetG(color)<100&&SkColorGetB(color)<100){sumX+=(x+.5)/3;sumY+=(y+.5)/3;++count;}}
    expect(count>50&&std::hypot(sumX/std::max(1.,count)-expected.x,sumY/std::max(1.,count)-expected.y)<.4,"rasterized item ink inherits the exact glass contact affine");
    UIGestureRecognizerTestHarness::send(window,touch,event,UITouchPhase::cancelled,contact);GlassInteractionTestHarness::advance(glass,2);
    if(out)out<<"]}\n";
    window->setRootViewController(nullptr);
}
void standalone(){
    operation="standalone";auto bar=new_shared<UINavigationBar>(NXRect{0,0,402,54});auto delegate=std::make_shared<BarDelegate>();bar->setDelegate(delegate);
    expect(!bar->topItem()&&!bar->backItem(),"empty standalone bar defaults");
    auto r=std::make_shared<UINavigationItem>("R"),d=std::make_shared<UINavigationItem>("D");
    bar->pushNavigationItem(r,false);bar->pushNavigationItem(d,false);delegate->veto=true;
    expect(bar->popNavigationItem(false)==d&&bar->items().size()==2,"a vetoed standalone pop returns the attempted item without removing it, like UIKit");
    delegate->veto=false;expect(bar->popNavigationItem(false)==d&&bar->popNavigationItem(false)==r&&bar->items().empty(),"standalone bars can pop their final item");
    const auto count=events.size();bar->setItems({r,d});expect(events.size()==count&&bar->backItem()==r,"setItems does not issue push/pop delegate notifications");
    expect(rejects([&]{bar->setItems({d,d});})&&bar->items().size()==2,"duplicate bar items reject atomically");
}
void opticalMotion(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<Context>& ctx) {
    ctx->size={402,874};ctx->density=3;ctx->dark=true;
    auto root=new_shared<UIViewController>();root->view()->setBackgroundColor(UIColor::black);
    root->navigationItem()->setRightBarButtonItems(groupingItems("pair"));
    auto nav=new_shared<UINavigationController>(root);window->setRootViewController(nav);window->makeKeyAndVisible();frame();settle();UIGestureRecognizerTestHarness::touch(window);
    const auto bar=nav->navigationBar();auto& state=Internal::NavigationBarAccess::state(*bar);
    auto tick=[&](Timer start,double seconds) {
        const long us=start.startTime.tv_usec+long(seconds*1000000);start.startTime.tv_sec+=us/1000000;start.startTime.tv_usec=us%1000000;
        std::function<void(const std::shared_ptr<CALayer>&)> visit=[&](const auto& layer){const auto children=layer->sublayers();layer->animateAt(start);for(const auto& child:children)visit(child);};visit(window->layer());
    };
    auto render=[&] {
        auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1206,240));auto canvas=surface->getCanvas();canvas->clear(SK_ColorBLACK);canvas->scale(3,3);canvas->translate(0,12-bar->frame().minY());
        UITraitCollection::setCurrent(window->traitCollection());bar->layer()->presentationOrSelf()->skiaRender(canvas);
        SkBitmap result;result.allocN32Pixels(1206,240);surface->readPixels(result,0,0);return result;
    };
    const auto carrier=state.right.groups.front().view;
    int duringTail=0;auto target=new_shared<UIViewController>();auto targetItems=groupingItems("fixed-0");targetItems.front()->setPrimaryAction(UIAction("Add",[&]{++duringTail;}));target->navigationItem()->setRightBarButtonItems(targetItems);
    const auto start=Timer();nav->pushViewController(target,true);window->drawAndLayoutTreeIfNeeded();
    tick(start,.22);const auto pulse=carrier->layer()->presentationOrSelf();
    expect(pulse->bounds().height()*NXTransform3DGetAffineTransform(pulse->transform()).m22>50,"native split has a visible surface inflation before pinching apart");
    tick(start,.25);auto pixels=render();
    expect(SkColorGetR(pixels.getColor(333*3,34*3))<2,"two-item split has the native clear center gap by the quarter-second checkpoint");
    tick(start,1.0/3);pixels=render();
    expect(SkColorGetR(pixels.getColor(336*3,34*3))<2,"split neck has detached by the native one-third-second checkpoint");
    tick(start,.45);const auto tail=carrier->layer()->presentationOrSelf();
    expect(tail->bounds().height()*NXTransform3DGetAffineTransform(tail->transform()).m22<44,"surface spring undershoots before settling");
    tick(start,.65);DispatchQueueTestHarness::pump();const auto control=state.right.buttons.front()->control;const auto contact=control->convertToView({control->bounds().width()*.5f,22},window);
    auto [touch,event]=UIGestureRecognizerTestHarness::begin(window,contact);UIGestureRecognizerTestHarness::send(window,touch,event,UITouchPhase::ended,contact);
    expect(duringTail==1,"navigation controls accept routed input while the residual optical spring is still running");
    for(const auto& glass:state.right.backgrounds)GlassInteractionTestHarness::advance(glass,2);
    tick(start,1.05);pixels=render();
    expect(SkColorGetR(pixels.getColor(336*3,34*3))<2,"settled capsules have a clear gap after their SDF neck separates");
    expect(!carrier->layer()->presentation()&&carrier->transform().isIdentity(),"surface pulse finishes at its model geometry without leaving a transform");
    // A clear center gap alone is insufficient: a broad blend kernel can
    // still pull the facing contours outward without making a full bridge.
    const auto effect=state.glass->effect();auto unmerged=effect;unmerged.setSpacing(0);state.glass->setEffect(unmerged);const auto independent=render();state.glass->setEffect(effect);
    int maximumContourShift=0;
    for(int y=36;y<168;++y)for(const auto range:{std::pair{280*3,336*3},std::pair{336*3,392*3}}) {
        auto edges=[&](const SkBitmap& b){int first=range.second,last=range.first;for(int x=range.first;x<range.second;++x)if(SkColorGetR(b.getColor(x,y))>8){first=std::min(first,x);last=std::max(last,x);}return std::pair{first,last};};
        const auto a=edges(pixels),b=edges(independent);maximumContourShift=std::max({maximumContourShift,std::abs(a.first-b.first),std::abs(a.second-b.second)});
    }
    expect(maximumContourShift<=1,"resting navigation capsules keep their independent round silhouettes within one raster pixel");
    settle();
    nav->setViewControllers({root},false);root->navigationItem()->setRightBarButtonItems(groupingItems("three"));frame();settle();
    const auto threeCarrier=state.right.groups.front().view;
    auto splitThree=new_shared<UIViewController>();splitThree->navigationItem()->setRightBarButtonItems(groupingItems("fixed-0"));
    const auto threeStart=Timer();nav->pushViewController(splitThree,true);frame();tick(threeStart,.23);
    expect(state.right.groups.front().view==threeCarrier,"removing the visual-leading item retains the trailing array-first carrier");
    pixels=render();expect(SkColorGetR(pixels.getColor(336*3,34*3))>8,"three-item removal keeps a real SDF join while its new carrier separates");settle();
    nav->setViewControllers({root},false);root->navigationItem()->setRightBarButtonItems(groupingItems("three"));frame();settle();
    const auto retainedTextInk=state.right.buttons.front()->ink;
    auto textScreen=new_shared<UIViewController>();textScreen->navigationItem()->setRightBarButtonItems(groupingItems("text-pair"));
    const auto textStart=Timer();nav->pushViewController(textScreen,true);frame();tick(textStart,.05);
    expect(state.right.buttons.front()->ink==retainedTextInk&&retainedTextInk->layer()->presentationOrSelf()->opacity()>.99,"retained array-first image-to-text item updates immediately while the other group separates");
    tick(textStart,.18);const auto movedTextInk=std::dynamic_pointer_cast<Internal::NavigationInkLayer>(state.right.buttons.back()->ink->layer()->presentationOrSelf());
    expect(movedTextInk&&movedTextInk->blur()>2,"text entering the new glass carrier receives the soft reveal");settle();
    // An image update with the same identifier is immediate in native UIKit.
    nav->setViewControllers({root},false);root->navigationItem()->setRightBarButtonItems(groupingItems("single"));
    auto same=new_shared<UIViewController>();auto matched=groupingItems("single");matched.front()->setImage(UIImage::systemImageNamed("trash",20,400,3));same->navigationItem()->setRightBarButtonItems(matched);
    const auto oldInk=state.right.buttons.front()->ink;nav->pushViewController(same,true);
    expect(state.right.buttons.front()->ink==oldInk&&oldInk->alpha()==1&&!oldInk->layer()->presentation(),"matched identifiers replace their image immediately without an invented blur transition");settle();
    auto different=new_shared<UIViewController>();auto unmatched=groupingItems("single");unmatched.front()->setIdentifier("replacement");different->navigationItem()->setRightBarButtonItems(unmatched);
    const auto replacementStart=Timer();nav->pushViewController(different,true);window->drawAndLayoutTreeIfNeeded();tick(replacementStart,.18);
    const auto ink=std::dynamic_pointer_cast<Internal::NavigationInkLayer>(state.right.buttons.front()->ink->layer()->presentationOrSelf());
    expect(ink&&ink->blur()>2,"an unmatched replacement uses a soft foreground reveal");tick(replacementStart,1.05);
    expect(!state.right.buttons.front()->ink->layer()->presentation(),"replacement ink resolves sharply and releases its filter presentation");settle();
    // Retarget before the previous optical tail ends. Retired material cannot
    // remain in the next union or survive an immediate nonanimated update.
    auto item=bar->topItem();item->setRightBarButtonItems(groupingItems("fixed-0"),true);const auto joinStart=Timer();item->setRightBarButtonItems(groupingItems("pair"),true);
    expect(state.glass->contentView()->subviews().size()>state.right.groups.size()+state.left.groups.size(),"join keeps redundant material alive during absorption");
    tick(joinStart,.05);expect(state.glass->contentView()->subviews().size()==state.right.groups.size()+state.left.groups.size(),"native join absorbs its redundant glass within the first 50 ms");
    item->setRightBarButtonItems(groupingItems("single"),false);
    expect(state.glass->contentView()->subviews().size()==state.right.groups.size()+state.left.groups.size(),"retargeting removes superseded glass before the next union");settle();
    // Raster evidence for foreground-only filtering: a sibling marker remains
    // byte-identical while ink spreads beyond its original silhouette.
    auto host=new_shared<UIView>(NXRect{0,0,80,44});auto filter=new_shared<Internal::NavigationInkView>();filter->setFrame({0,0,80,44});host->addSubview(filter);
    auto mark=new_shared<UIView>(NXRect{20,12,4,20});mark->setBackgroundColor(UIColor::white);filter->addSubview(mark);
    auto sibling=new_shared<UIView>(NXRect{60,12,4,20});sibling->setBackgroundColor(UIColor::systemRed);host->addSubview(sibling);
    auto raster=[&]{auto s=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(80,44));s->getCanvas()->clear(SK_ColorBLACK);host->layer()->skiaRender(s->getCanvas());SkBitmap b;b.allocN32Pixels(80,44);s->readPixels(b,0,0);return b;};
    const auto sharp=raster();Internal::navigationInkBlur(filter,3,0,.3);filter->layer()->animateAt(Timer());const auto soft=raster();
    expect(SkColorGetR(soft.getColor(18,22))>SkColorGetR(sharp.getColor(18,22))+10&&SkColorGetR(soft.getColor(22,22))<SkColorGetR(sharp.getColor(22,22))-30,"foreground blur spreads and softens actual raster ink");
    bool unchanged=true;for(int y=0;y<44;++y)for(int x=58;x<67;++x)unchanged&=sharp.getColor(x,y)==soft.getColor(x,y);
    expect(unchanged,"navigation ink blur does not filter its sibling content or backing material");
    // Native eight-layout cycle: supplementing an existing Back delays the
    // star until the Back surface has inflated; root entry has no such wait.
    auto screen=[&](bool star){auto value=new_shared<UIViewController>();value->setTitle(star?"Leading":"Shared");value->navigationItem()->setRightBarButtonItems(groupingItems("pair"));if(star){auto item=std::make_shared<UIBarButtonItem>(UIImage::systemImageNamed("star",20,400,3),UIBarButtonItemStyle::plain);item->setIdentifier("favorite");value->navigationItem()->setLeftItemsSupplementBackButton(true);value->navigationItem()->setLeftBarButtonItem(item);}return value;};
    auto source=screen(false),leading=screen(true);nav->setViewControllers({root,source},false);frame();settle();
    const auto oldTitleHost=state.titleInk;const auto retained=state.left.groups.front().view;
    const auto leadingStart=Timer();nav->pushViewController(leading,true);frame();const auto star=state.left.groups.back().view;
    tick(leadingStart,.1);
    expect(std::abs(NXTransform3DGetAffineTransform(star->layer()->presentationOrSelf()->transform()).m11)<.001f,
           "supplemental leading glass waits at zero size for the Back morph");
    expect(star->layer()->presentationOrSelf()->opacity()==1&&state.left.buttons.back()->ink->layer()->presentationOrSelf()->opacity()<.02,
           "leading birth keeps glass opaque and delays its ink separately");
    const auto incomingHost=leading->view()->superview().lock();const auto travel=NXTransform3DGetAffineTransform(incomingHost->layer()->presentationOrSelf()->transform()).tX;
    expect(!leading->view()->isActionDispatchAllowed(),"controller content rejects actions during its native transition interval");
    expect(travel>160&&travel<270,"native content spring covers roughly half the width in the first 100 ms");
    const auto titleTravel=NXTransform3DGetAffineTransform(state.titleInk->layer()->presentationOrSelf()->transform()).tX;
    expect(std::abs(titleTravel-travel)<4,"incoming title and content share their slide clock");
    expect(oldTitleHost->clipsToBounds()&&oldTitleHost->layer()->presentationOrSelf()->bounds().width()<340,"outgoing title is wiped by the incoming content boundary");
    tick(leadingStart,.22);const auto backPose=NXTransform3DGetAffineTransform(retained->layer()->presentationOrSelf()->transform());
    expect(backPose.m11>1.15f&&backPose.tX>3,"supplemental leading transition inflates Back about its leading top edge");
    const auto backBlur=std::dynamic_pointer_cast<Internal::NavigationInkLayer>(state.left.buttons.front()->ink->layer()->presentationOrSelf());
    expect(backBlur&&backBlur->blur()>5,"Back ink softens during the leading glass split");
    // A fading foreground material used to punch a black hole into Back.
    // Compare only material, with both glyphs hidden, at an interior overlap.
    auto noMaterialHole=[&](const std::shared_ptr<UIView>& glass) {
        std::vector<std::shared_ptr<UIView>> ink;
        std::function<void(const std::shared_ptr<UIView>&)> hide=[&](const auto& view){
            if(std::dynamic_pointer_cast<Internal::NavigationInkView>(view)&&!view->isHidden()){ink.push_back(view);view->setHidden(true);}
            for(const auto& child:view->subviews())hide(child);
        };hide(bar);
        const auto pose=glass->layer()->presentationOrSelf();
        const auto x=int(std::round(pose->position().x*3)),y=int(std::round((pose->position().y+12)*3));
        const auto overlapping=render();glass->setHidden(true);const auto backOnly=render();glass->setHidden(false);
        int loss=0;for(int dy=-2;dy<=2;++dy)for(int dx=-2;dx<=2;++dx)
            loss=std::max(loss,int(SkColorGetR(backOnly.getColor(x+dx,y+dy)))-int(SkColorGetR(overlapping.getColor(x+dx,y+dy))));
        for(const auto& view:ink)view->setHidden(false);
        expect(loss<=4,"overlapping leading material does not expose a dark hole through Back");
    };
    tick(leadingStart,.3);noMaterialHole(star);
    tick(leadingStart,.4);const auto starPose=star->layer()->presentationOrSelf();
    expect(starPose->position().x>70&&starPose->position().x<88&&NXTransform3DGetAffineTransform(starPose->transform()).m11>.6f,"leading glass travels from Back before reaching its final center");
    tick(leadingStart,.55);DispatchQueueTestHarness::pump();expect(!source->view()->window()&&!state.transitioning&&leading->view()->isActionDispatchAllowed(),"content completion and navigation actions resume at the native half-second boundary");
    const auto settledBack=std::dynamic_pointer_cast<Internal::NavigationInkLayer>(state.left.buttons.front()->ink->layer()->presentationOrSelf());
    expect(settledBack&&settledBack->blur()<.01,"Back ink is sharp when the content slide completes");
    tick(leadingStart,1.05);DispatchQueueTestHarness::pump();settle();
    const auto returnStart=Timer();nav->popViewController(true);frame();tick(returnStart,.2);
    expect(star->isDescendantOf(bar)&&NXTransform3DGetAffineTransform(star->layer()->presentationOrSelf()->transform()).m11>.3,
           "leading removal absorbs material over hundreds of milliseconds instead of the trailing fast merge");
    tick(returnStart,.25);noMaterialHole(star);
    tick(returnStart,.45);expect(std::abs(NXTransform3DGetAffineTransform(star->layer()->presentationOrSelf()->transform()).m11)<.001f,
                              "absorbed supplemental glass disappears geometrically");settle();
    auto unchangedScreen=screen(false);nav->pushViewController(unchangedScreen,true);frame();const auto unchangedStart=Timer();tick(unchangedStart,.22);
    expect(NXTransform3DGetAffineTransform(state.left.groups.front().view->layer()->presentationOrSelf()->transform()).isIdentity(),"unchanged automatic Back has no invented surface pulse");settle();
    root->navigationItem()->setLeftBarButtonItem(std::make_shared<UIBarButtonItem>(UIImage::systemImageNamed("chevron.left",20,600,3),UIBarButtonItemStyle::plain));
    nav->setViewControllers({root},false);auto rootLeading=screen(true);const auto entryStart=Timer();nav->pushViewController(rootLeading,true);frame();tick(entryStart,.2);
    expect(NXTransform3DGetAffineTransform(state.left.groups.back().view->layer()->presentationOrSelf()->transform()).m11>.6,
           "root-to-leading transition reveals the star earlier than a supplemental Back change");settle();
    const auto rootStar=state.left.groups.back().view;const auto exitStart=Timer();nav->popViewController(true);frame();tick(exitStart,.18);
    expect(NXTransform3DGetAffineTransform(rootStar->layer()->presentationOrSelf()->transform()).m11>.8,
           "pop to the custom root Back delays supplemental absorption");
    nav->setViewControllers({root},false);frame();settle();
    expect(!state.titleInk->layer()->presentation()&&state.titleInk->transform().isIdentity(),"nonanimated retarget clears title motion and leaves correct model geometry");
    window->setRootViewController(nullptr);settle();
}
void navigationInk(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<Context>& ctx) {
    const auto originalDark=ctx->dark;const auto originalDensity=ctx->density;
    auto root=new_shared<UIViewController>(),detail=new_shared<UIViewController>();
    auto image=std::make_shared<UIBarButtonItem>(UIBarButtonSystemItem::add);
    auto text=std::make_shared<UIBarButtonItem>("Edit",UIBarButtonItemStyle::plain);
    auto leading=std::make_shared<UIBarButtonItem>(UIBarButtonSystemItem::search);
    detail->navigationItem()->setRightBarButtonItems({image,text});
    detail->navigationItem()->setLeftBarButtonItem(leading);
    detail->navigationItem()->setLeftItemsSupplementBackButton(true);
    auto nav=new_shared<UINavigationController>(root);nav->setViewControllers({root,detail},false);
    window->setRootViewController(nav);window->makeKeyAndVisible();frame();settle();
    auto bar=nav->navigationBar();auto& state=Internal::NavigationBarAccess::state(*bar);
    auto ordinaryInk=[&] {
        expect(state.left.buttons.front()->back,"tint fixture includes automatic Back");
        for(const auto* side:{&state.left,&state.right})for(const auto& entry:side->buttons) {
            expect(entry->image->tintColor()==UIColor::label&&entry->label->textColor()==UIColor::label,
                   "plain image, text, supplemental and Back items use label ink");
            const auto resolved=entry->image->tintColor().resolvedColorWithTraitCollection(bar->traitCollection());
            expect(resolved.raw()==(ctx->dark?UIColor::white:UIColor::black).raw(),"label ink resolves for the current theme");
        }
    };
    for(float scale:{1.f,2.f,3.f})for(bool dark:{false,true}) {
        ctx->density=scale;ctx->dark=dark;frame();
        window->setTintColor(UIColor::systemRed);ordinaryInk();
        bar->setTintColor(UIColor::systemBlue);ordinaryInk();
        image->setTintColor(UIColor::systemPink);text->setTintColor(UIColor::systemGreen);
        leading->setTintColor(UIColor::systemOrange);
        expect(state.right.buttons[0]->image->tintColor()==UIColor::systemPink&&
               state.right.buttons[1]->label->textColor()==UIColor::systemGreen&&
               state.left.buttons[1]->image->tintColor()==UIColor::systemOrange,
               "explicit item tints update image and text ink live");
        expect(state.left.buttons.front()->image->tintColor()==UIColor::label,"explicit sibling tint leaves Back neutral");
        image->setTintColor(std::nullopt);text->setTintColor(std::nullopt);leading->setTintColor(std::nullopt);ordinaryInk();
        image->setStyle(UIBarButtonItemStyle::prominent);
        expect(state.right.buttons[0]->image->tintColor()==UIColor::white&&
               state.right.backgrounds[0]->glassEffect()->tintColor()==UIColor::systemBlue,
               "prominent action retains white ink and the bar accent fill");
        image->setTintColor(UIColor::systemPink);
        expect(state.right.buttons[0]->image->tintColor()==UIColor::white&&
               state.right.backgrounds[0]->glassEffect()->tintColor()==UIColor::systemPink,
               "explicit prominent tint changes its glass fill while keeping white ink");
        image->setTintColor(std::nullopt);image->setStyle(UIBarButtonItemStyle::plain);ordinaryInk();
        bar->setTintColor(std::nullopt);ordinaryInk();window->setTintColor(std::nullopt);
    }
    window->setRootViewController(nullptr);ctx->dark=originalDark;ctx->density=originalDensity;frame();settle();
}
void stableBack(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<Context>& ctx) {
    const auto originalDark=ctx->dark;const auto originalDensity=ctx->density;const auto originalSize=ctx->size;
    for(const NXKit::NXSize size:{NXKit::NXSize{402,874},NXKit::NXSize{844,390}})for(float density:{1.f,2.f,3.f})for(bool dark:{false,true})for(bool supplemented:{false,true}) {
        ctx->size=size;ctx->density=density;ctx->dark=dark;
        auto root=new_shared<Screen>("Root"),source=new_shared<Screen>("Source");
        auto configureLeading=[&](const auto& screen){if(supplemented){auto star=std::make_shared<UIBarButtonItem>(UIImage::systemImageNamed("star",20,400,density),UIBarButtonItemStyle::plain);star->setIdentifier("stable.leading");screen->navigationItem()->setLeftItemsSupplementBackButton(true);screen->navigationItem()->setLeftBarButtonItem(star);}};
        configureLeading(source);
        auto nav=new_shared<UINavigationController>(root);nav->setViewControllers({root,source},false);
        window->setRootViewController(nav);window->makeKeyAndVisible();window->drawAndLayoutTreeIfNeeded();frame();settle();
        auto bar=nav->navigationBar();auto& state=Internal::NavigationBarAccess::state(*bar);
        const auto back=state.left.buttons.front();const auto glass=state.left.groups.front().view;
        const auto item=back->item;const auto ink=back->ink;const auto control=back->control;
        const auto identifier=item->identifier();
        expect(identifier.has_value(),"automatic Back has an explicit stable identifier");
        const auto bounds=glass->layer()->bounds();const auto position=glass->layer()->position();
        const auto controlBounds=control->layer()->bounds();const auto controlPosition=control->layer()->position();
        auto inspect=[&] {
            expect(state.left.buttons.front()==back&&state.left.groups.front().view==glass&&back->control==control&&back->ink==ink&&back->item==item&&item->identifier()==identifier,"unchanged Back retains item, control, ink and glass identity");
            const auto material=glass->layer()->presentationOrSelf(),button=control->layer()->presentationOrSelf();
            const auto soft=std::dynamic_pointer_cast<Internal::NavigationInkLayer>(ink->layer()->presentationOrSelf());
            expect(material->bounds()==bounds&&material->position()==position&&material->transform()==NXTransform3D::identity&&material->opacity()==1,"unchanged Back glass never moves, resizes, pulses or fades");
            expect(button->bounds()==controlBounds&&button->position()==controlPosition&&button->transform()==NXTransform3D::identity&&ink->layer()->presentationOrSelf()->opacity()==1&&soft&&soft->blur()==0,"unchanged Back glyph stays sharp and stationary");
        };
        for(int cycle=0;cycle<3;++cycle) {
            auto target=new_shared<Screen>(cycle%2?"Short":"A longer title with different trailing items");
            configureLeading(target);
            target->navigationItem()->setRightBarButtonItems(groupingItems(cycle%2?"prominent":"pair"));
            for(bool pop:{false,true}) {
                const auto origin=Timer();if(pop)nav->popViewController(true);else nav->pushViewController(target,true);
                inspect();
                for(int i=0;i<=132;++i) {
                    auto now=origin;const auto micros=origin.startTime.tv_usec+i*1000000L/120;now.startTime.tv_sec+=micros/1000000;now.startTime.tv_usec=micros%1000000;
                    std::function<void(const std::shared_ptr<CALayer>&)> tick=[&](const auto& layer){const auto children=layer->sublayers();layer->animateAt(now);for(const auto& child:children)tick(child);};tick(window->layer());DispatchQueueTestHarness::pump();inspect();
                }
                settle();inspect();
            }
        }
        int invoked=0;auto custom=std::make_shared<UIBarButtonItem>("Custom",UIBarButtonItemStyle::plain,UIAction("Custom",[&]{++invoked;}));custom->setIdentifier(identifier);
        auto replacement=new_shared<Screen>("Custom leading");replacement->navigationItem()->setLeftBarButtonItem(custom);nav->pushViewController(replacement,false);
        const auto customControl=state.left.buttons.front()->control;
        expect(customControl!=control&&!state.left.buttons.front()->back,"a custom identifier cannot reuse automatic Back's control");
        customControl->performPrimaryAction();expect(invoked==1&&nav->topViewController()==replacement,"a colliding custom identifier keeps its own action");
        nav->popViewController(false);
        expect(state.left.buttons.front()->back&&state.left.buttons.front()->item==item&&item->identifier()==identifier&&state.left.buttons.front()->control!=customControl,"Back keeps its semantic identity after a custom leading replacement");
        window->setRootViewController(nullptr);settle();
    }
    ctx->dark=originalDark;ctx->density=originalDensity;ctx->size=originalSize;frame();
}
}
int main(int argc,char**argv){
    auto ctx=std::make_shared<Context>();if(argc>3)ctx->dark=std::string(argv[3])=="dark";SkiaCtx::_main=ctx;UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
    auto a=new_shared<Screen>("Home"),b=new_shared<Screen>("Details"),c=new_shared<Screen>("Editor");
    int invoked=0;
    auto more=std::make_shared<UIBarButtonItem>(UIImage::systemImageNamed("ellipsis",20,400,3),UIBarButtonItemStyle::plain,UIAction("More",[&]{++invoked;}));
    a->navigationItem()->setLeftBarButtonItem(std::make_shared<UIBarButtonItem>("Close",UIBarButtonItemStyle::plain));
    a->navigationItem()->setRightBarButtonItems({std::make_shared<UIBarButtonItem>(UIBarButtonSystemItem::add),more});b->navigationItem()->setRightBarButtonItems({more});c->navigationItem()->setRightBarButtonItem(std::make_shared<UIBarButtonItem>(UIBarButtonSystemItem::done));
    auto nav=new_shared<UINavigationController>(a);auto delegate=std::make_shared<Delegate>();nav->setDelegate(delegate);
    expect(a->navigationController()==nav&&!a->viewIsLoaded()&&!nav->viewIsLoaded(),"navigation containment precedes loading");
    auto window=new_shared<UIWindow>();UIApplication::shared->delegate->window=window;window->setRootViewController(nav);window->makeKeyAndVisible();frame();
    auto bar=nav->navigationBar();auto& state=Internal::NavigationBarAccess::state(*bar);
    expect(bar->delegate()==nav&&bar->items().size()==1,"controller owns and synchronizes bar delegate/items");
    expect(rejects([&]{bar->setItems({});})&&rejects([&]{bar->setDelegate(nullptr);}),"managed bar mutations route through the controller");
    expect(a->view()->safeAreaInsets().top==116,"inline bar reservation matches native portrait UIKit");
    expect(state.right.buttons[0]->control->frame().minX()>state.right.buttons[1]->control->frame().minX(),"right bar button array is rightmost first");
    state.right.buttons[1]->control->performPrimaryAction();expect(invoked==1,"bar action invokes the latest closure");
    more->setEnabled(false);state.right.buttons[1]->control->performPrimaryAction();expect(invoked==1&&!state.right.buttons[1]->control->canBecomeFocused(),"disabled bar buttons neither activate nor focus");more->setEnabled(true);
    std::ofstream report;if(argc>1)report.open(argv[1]);
    bool first=true;auto capture=[&](const char* name){frame();if(!report)return;if(!first)report<<',';first=false;report<<"{\"name\":\""<<name<<"\",\"stack\":[";bool comma=false;for(const auto& child:nav->viewControllers()){if(comma)report<<',';comma=true;report<<'"'<<child->title()<<'"';}report<<"],\"top\":\""<<(nav->topViewController()?nav->topViewController()->title():"nil")<<"\",\"bar\":";rect(report,bar->frame());const auto safe=nav->topViewController()->view()->safeAreaInsets();report<<",\"safe\":["<<safe.top<<','<<safe.bottom<<"],\"groups\":[";bool delimiter=false;for(const auto* side:{&state.left,&state.right})for(const auto& bg:side->backgrounds)if(!bg->isHidden()&&bg->alpha()>.01f){if(delimiter)report<<',';delimiter=true;rect(report,bg->frame());}report<<"]}";};
    if(report)report<<"{\"states\":[";
    UIGestureRecognizerTestHarness::touch(window);settle();
    capture("root");if(argc>2)snapshot(window,std::string(argv[2])+"/root.ppm",2);
    standalone();
    struct Motion {double t;NXRect left,right;};std::vector<Motion> motion,popMotion;
    const auto originalLeft=state.left.backgrounds.front()->frame(),originalRight=state.right.backgrounds.front()->frame();
    operation="push";const auto background=state.right.backgrounds.front();const auto oldWidth=background->frame().width();
    const auto transform=NXAffineTransform::scale(.97f);a->view()->setTransform(transform);
    const auto motionOrigin=Timer();nav->pushViewController(b,true);expect(nav->topViewController()==b&&nav->visibleViewController()==b&&a->disappearances==0,"animated push updates stack before appearance completion");
    for(const auto& screen:{a,b}) {
        const auto host=screen->view()->superview().lock();
        expect(host->clipsToBounds()&&host->effectiveRadius(UIRectCorner::topLeft)==62&&host->effectiveRadius(UIRectCorner::bottomRight)==38,"animated push cards preserve independent screen corners");
    }
    expect(state.right.backgrounds.front()==background&&background->layer()->presentation(),"morph retains and animates the trailing glass carrier");
    expect(background->frame().width()<oldWidth&&background->layer()->presentation()->bounds().width()>background->frame().width(),"presentation width starts at the outgoing shared capsule");
    auto sampleMotion=[&](Timer origin,std::vector<Motion>& output){
        if(report)for(int i=0;i<=120;++i) {
        const auto t=i/120.0;auto now=origin;const auto micros=long(t*1000000)+now.startTime.tv_usec;
        now.startTime.tv_sec+=micros/1000000;now.startTime.tv_usec=micros%1000000;
        const auto left=state.left.backgrounds.front()->layer(),right=state.right.backgrounds.front()->layer();
        left->animateAt(now);right->animateAt(now);
        auto presentedFrame=[](const auto& layer){const auto p=layer->presentationOrSelf();const auto size=p->bounds().size;const auto pos=p->position(),anchor=p->anchorPoint();return NXRect{pos.x-size.width*anchor.x,pos.y-size.height*anchor.y,size.width,size.height};};
        output.push_back({t,presentedFrame(left),presentedFrame(right)});
    }
    };
    sampleMotion(motionOrigin,motion);
    capture("push-immediate");settle();capture("pushed");if(argc>2)snapshot(window,std::string(argv[2])+"/pushed.ppm",2);
    expect(!b->view()->superview().lock()->clipsToBounds()&&b->view()->superview().lock()->effectiveRadius(UIRectCorner::allCorners)==0,"push completion clears temporary card styling");
    expect(a->view()->transform()==transform&&a->disappearances==1&&b->appearances==1,"container transitions preserve content transforms and balance appearance");
    a->view()->setTransform(NXAffineTransform::identity);
    operation="push-prominent";nav->pushViewController(c,true);settle();capture("editor");
    operation="pop";expect(nav->popViewController(true)==c,"pop returns its removed controller");capture("pop-immediate");
    for(const auto& screen:{b,c})expect(screen->view()->superview().lock()->clipsToBounds()&&screen->view()->superview().lock()->effectiveRadius(UIRectCorner::topLeft)==62,"animated pop clips both moving cards");
    settle();capture("popped");
    expect(c->parent().expired()&&b->navigationController()==nav,"completed pop detaches only removed children");
    const auto popLeft=state.left.backgrounds.front()->frame(),popRight=state.right.backgrounds.front()->frame();
    operation="pop-root";const auto popOrigin=Timer();nav->popToRootViewController(true);sampleMotion(popOrigin,popMotion);settle();capture("root-restored");expect(!nav->popViewController(false),"a navigation controller preserves its root");
    operation="replace";nav->setViewControllers({a,c},false);capture("replaced");
    operation="hidden";nav->setNavigationBarHidden(true);capture("hidden");
    operation="shown";nav->setNavigationBarHidden(false);capture("shown");
    if(report){report<<"],\"events\":[";bool comma=false;for(const auto& [op,event]:events){if(comma)report<<',';comma=true;report<<"{\"operation\":\""<<op<<"\",\"event\":\""<<event<<"\"}";}report<<"],\"motionFrom\":[";rect(report,originalLeft);report<<',';rect(report,originalRight);report<<"],\"motion\":[";
        bool delimiter=false;for(const auto& sample:motion){if(delimiter)report<<',';delimiter=true;report<<"{\"t\":"<<sample.t<<",\"groups\":[";rect(report,sample.left);report<<',';rect(report,sample.right);report<<"]}";}report<<"],\"popMotionFrom\":[";rect(report,popLeft);report<<',';rect(report,popRight);report<<"],\"popMotion\":[";
        delimiter=false;for(const auto& sample:popMotion){if(delimiter)report<<',';delimiter=true;report<<"{\"t\":"<<sample.t<<",\"groups\":[";rect(report,sample.left);report<<',';rect(report,sample.right);report<<"]}";}report<<"]}\n";report.close();}
    expect(rejects([&]{nav->pushViewController(c,false);})&&rejects([&]{nav->pushViewController(nav,false);}),"duplicates and cycles reject before mutation");
    a->onDisappear=[&]{a->onDisappear=nullptr;nav->pushViewController(c,true);};nav->setViewControllers({a},false);nav->pushViewController(b,true);settle();
    expect(nav->topViewController()==c&&nav->viewControllers().size()==3,"reentrant lifecycle pushes serialize and preserve requests");
    nav->popToRootViewController(false);nav->pushViewController(b,true);expect(nav->popViewController(true)==b,"queued pop reserves the requested stack result");expect(nav->topViewController()==b,"queued pop does not mutate the active push");settle();expect(nav->topViewController()==a,"queued pop drains after push completion");
    auto customTitle=new_shared<UILabel>();customTitle->setText("Custom title");a->navigationItem()->setTitleView(customTitle);frame();
    expect(customTitle->superview().lock()==bar&&customTitle->frame().width()>0,"custom title views mount and lay out in the bar");
    a->navigationItem()->setTitleView(nullptr);expect(customTitle->superview().expired(),"clearing a title view detaches it");
    auto custom=new_shared<UIButton>();custom->setFrame({0,0,80,44});auto customItem=std::make_shared<UIBarButtonItem>(custom);
    a->navigationItem()->setLeftBarButtonItem(customItem);frame();expect(custom->isDescendantOf(state.left.buttons[0]->control),"custom bar button views mount through their item");
    window->setTintColor(UIColor::systemRed);expect(state.right.buttons[0]->image->tintColor()==UIColor::label,"ancestor accent does not replace ordinary navigation label ink");window->setTintColor(std::nullopt);
    a->show(b);settle();expect(nav->topViewController()==b,"child show routes through its UIKit navigation container");
    expect(nav->registeredAction("UIKit.navigationController.back")->canPerform(),"semantic Back is discoverable through Core responders");
    b->navigationItem()->setHidesBackButton(true);expect(state.left.buttons.empty(),"hidesBackButton removes the automatic back presentation");
    auto supplemental=std::make_shared<UIBarButtonItem>("Edit",UIBarButtonItemStyle::plain);
    b->navigationItem()->setLeftBarButtonItem(supplemental);b->navigationItem()->setHidesBackButton(false);b->navigationItem()->setLeftItemsSupplementBackButton(true);
    expect(state.left.buttons.size()==2&&state.left.buttons.front()->back,"custom left items can supplement the automatic back button");
    b->navigationItem()->setLeftBarButtonItem(nullptr);b->navigationItem()->setLeftItemsSupplementBackButton(false);
    state.left.buttons.front()->control->performPrimaryAction();settle();
    expect(nav->topViewController()==a&&!nav->registeredAction("UIKit.navigationController.back")->canPerform(),"automatic bar Back pops and becomes unavailable at the root");
    a->setTitle("Updated");expect(a->navigationItem()->title()=="Updated"&&bar->topItem()->title()=="Updated","default navigation title updates live");a->navigationItem()->setTitle("Explicit");a->setTitle("Other");expect(bar->topItem()->title()=="Explicit","explicit navigation title survives controller title changes");
    const auto add=a->navigationItem()->rightBarButtonItems().front();
    const auto originalAction=add->primaryAction();int adds=0;add->setPrimaryAction(UIAction("Add",[&]{++adds;}));
    for(const auto style:{UIBarButtonItemStyle::plain,UIBarButtonItemStyle::prominent}) {
        add->setStyle(style);frame();
        for(size_t index:{size_t(0),size_t(1)}) {
            const auto control=state.right.buttons[index]->control;
            const auto point=control->convertToView({control->bounds().width()*.5f,22},window);
            const auto before=index==0?adds:invoked;
            auto [touch,event]=UIGestureRecognizerTestHarness::begin(window,point);
            int responding=0;
            for(const auto& glass:state.right.backgrounds){GlassInteractionTestHarness::advance(glass,.15);responding+=!glass->interactionTransform().isIdentity();}
            expect(responding==1,"routed button presses deform exactly their shared or prominent glass carrier");
            UIGestureRecognizerTestHarness::send(window,touch,event,UITouchPhase::ended,point);
            expect((index==0?adds:invoked)==before+1,"interactive glass preserves single button action delivery");
            for(const auto& glass:state.right.backgrounds){GlassInteractionTestHarness::advance(glass,2);expect(glass->interactionTransform().isIdentity(),"released navigation glass returns to rest");}
        }
    }
    const auto moreControl=state.right.buttons[1]->control;
    const auto contact=moreControl->convertToView({moreControl->bounds().width()*.5f,22},window);
    const auto beforeCancel=invoked;auto [contactTouch,contactEvent]=UIGestureRecognizerTestHarness::begin(window,contact);
    UIGestureRecognizerTestHarness::send(window,contactTouch,contactEvent,UITouchPhase::moved,contact+NXKit::NXPoint(-10,8));
    for(const auto& glass:state.right.backgrounds)GlassInteractionTestHarness::advance(glass,.15);
    expect(!state.right.backgrounds[1]->interactionTransform().isIdentity(),"dragged navigation presses retain material feedback");
    UIGestureRecognizerTestHarness::send(window,contactTouch,contactEvent,UITouchPhase::cancelled,contact);
    for(const auto& glass:state.right.backgrounds){GlassInteractionTestHarness::advance(glass,2);expect(glass->interactionTransform().isIdentity(),"cancelled navigation presses settle");}
    expect(invoked==beforeCancel,"cancelled material tracking does not invoke its button");
    more->setEnabled(false);auto [disabledTouch,disabledEvent]=UIGestureRecognizerTestHarness::begin(window,contact);
    for(const auto& glass:state.right.backgrounds){GlassInteractionTestHarness::advance(glass,.15);expect(glass->interactionTransform().isIdentity(),"disabled navigation buttons do not press their material");}
    UIGestureRecognizerTestHarness::send(window,disabledTouch,disabledEvent,UITouchPhase::ended,contact);more->setEnabled(true);
    add->setStyle(UIBarButtonItemStyle::plain);add->setPrimaryAction(originalAction);
    a->setAdditionalSafeAreaInsets({7,0,0,0});frame();expect(a->additionalSafeAreaInsets().top==7&&a->view()->safeAreaInsets().top==123,"navigation reservations preserve application insets");
    nav->setNavigationBarHidden(true,true);settle();frame();expect(a->view()->safeAreaInsets().top==69,"hiding chrome removes only the bar reservation");nav->setNavigationBarHidden(false,true);settle();frame();expect(!bar->isHidden()&&bar->alpha()==1,"animated show restores chrome visibility");
    for(float scale:{1.f,2.f,3.f})for(bool dark:{false,true}){ctx->density=scale;ctx->dark=dark;frame();bar->drawAndLayoutTreeIfNeeded();expect(bar->traitCollection()->displayScale()==scale,"bar inherits density and theme");}
    ctx->size={844,390};frame();expect(bar->frame().width()==844&&bar->sizeThatFits({844,390}).height==44,"compact-height layout uses the inline compact bar");
    window->setRootViewController(nullptr);UIApplication::shared->delegate->window.reset();std::weak_ptr<UINavigationController> weak=nav;nav.reset();expect(weak.expired()&&!bar->delegate(),"retaining a managed bar does not retain its controller");
    UIApplication::shared->delegate->window=window;grouping(window,ctx,argc>4?argv[4]:nullptr,argc>5?argv[5]:nullptr);opticalMotion(window,ctx);navigationInk(window,ctx);stableBack(window,ctx);UIApplication::shared->delegate->window.reset();
    UIApplication::shared.reset();SkiaCtx::_main.reset();if(!failures)std::cout<<"UIKit navigation tests passed\n";return failures?1:0;
}
