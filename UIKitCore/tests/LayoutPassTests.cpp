#include <UIApplication.h>
#include <SkiaCtx.h>
#include <UIViewController.h>
#include <YGLayout.h>
#include <CABasicAnimation.h>
#include "include/core/SkSurface.h"
#include "include/core/SkBitmap.h"
#include <cmath>
#include <iostream>
#include <cstring>
using namespace NXKit;
namespace NXKit {
void renderApplicationFrame();
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>){return true;}
class GlassMaterialTestHarness {
public:
    static auto traits(float scale){auto value=std::make_shared<UITraitCollection>();value->_displayScale=scale;return value;}
};
}
namespace {
int failures=0;
void expect(bool value,const char* text){if(!value){++failures;std::cerr<<"FAIL: "<<text<<'\n';}}
bool near(float a,float b){return std::abs(a-b)<.002f;}
class Context final:public SkiaCtx {
public:
    NXSize size{320,480};float scale=2;UIEdgeInsets safe{20,0,10,0};int presents=0;
    NXSize getSize()override{return size*_extraScaleFactor;}
    float getScaleFactor()override{return scale/_extraScaleFactor;}
    UIEdgeInsets deviceSafeAreaInsets()override{return {safe.top*_extraScaleFactor,safe.left*_extraScaleFactor,safe.bottom*_extraScaleFactor,safe.right*_extraScaleFactor};}
    sk_sp<SkSurface> surface;
    sk_sp<SkSurface> getBackbufferSurface()override{
        surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(std::lround(size.width*scale),std::lround(size.height*scale)));return surface;
    }
    void flushAndSubmit(sk_sp<SkSurface>)override{}
    void swapBuffers()override{++presents;}
    void sleepForNextFrame()override{}
    bool platformRunLoop(std::function<bool()>)override{return false;}
protected:
    skgpu::graphite::Context* graphiteContext()override{return nullptr;}
    skgpu::graphite::Recorder* graphiteRecorder()override{return nullptr;}
};
class PaintProbe final:public UIView {
public:
    NXSize laidOut,painted;int layouts=0,draws=0;
    PaintProbe(){setContentMode(UIViewContentMode::redraw);}
    void layoutSubviews()override{UIView::layoutSubviews();++layouts;laidOut=bounds().size;setNeedsDisplay();}
    void draw()override{++draws;painted=laidOut;setBackgroundColor(painted==bounds().size?UIColor::green:UIColor::red);}
};
class ManualHost final:public UIView {
public:
    std::shared_ptr<PaintProbe> child=new_shared<PaintProbe>();
    ManualHost(){addSubview(child);}
    void layoutSubviews()override{UIView::layoutSubviews();child->setFrame(bounds().insetBy({-3,-5,-7,-11}));}
};
class LateController final:public UIViewController {
public:
    float inset=17;
    bool appearedWithLayout=false;
    std::shared_ptr<ManualHost> host=new_shared<ManualHost>();
    void loadView()override{
        auto root=new_shared<UIView>();root->setAutolayoutEnabled(true);
        host->setAutolayoutEnabled(true);host->configureLayout([](const auto& y){y->setFlexGrow(1);});root->addSubview(host);setView(root);
    }
    void viewDidLayoutSubviews()override{
        UIViewController::viewDidLayoutSubviews();
        view()->configureLayout([&](const auto& y){y->setPaddingLeft({inset,YGUnitPoint});});
    }
    void viewDidAppear(bool animated)override{
        UIViewController::viewDidAppear(animated);
        appearedWithLayout=near(host->frame().minX(),inset)&&host->child->bounds().size==host->bounds().insetBy({-3,-5,-7,-11}).size;
    }
};
class FractionalRoot final:public UIView {
public:
    void scale(float value){const auto old=_traitCollection;_traitCollection=GlassMaterialTestHarness::traits(value);traitCollectionDidChange(old);}
};
class LayoutAction final:public UIView {
public:
    int calls=0;
    std::function<void()> action;
    void layoutSubviews()override{UIView::layoutSubviews();++calls;const auto callback=action;if(callback)callback();}
};
class BitmapProbe final:public UIView {
public:
    NXSize laidOut;
    void layoutSubviews()override{UIView::layoutSubviews();laidOut=bounds().size;}
    void draw()override{
        SkBitmap image;image.allocN32Pixels(1,1);image.eraseColor(laidOut.width>150?SK_ColorGREEN:SK_ColorRED);
        layer()->setContents(new_shared<CGImage>(image.asImage()));
        layer()->setContentsScale(bounds().width()>150?2:1);
    }
};
void firstPaintAndResize(const std::shared_ptr<Context>& context){
    auto window=new_shared<UIWindow>();auto controller=new_shared<LateController>();window->setRootViewController(controller);window->makeKeyAndVisible();
    renderApplicationFrame();
    auto probe=controller->host->child;
    expect(controller->appearedWithLayout,"initial appearance sees final safe areas and nested layout");
    expect(probe->painted==probe->bounds().size&&probe->draws==1,"first presented frame paints after final child layout");
    expect(near(controller->host->frame().minX(),17),"late controller Yoga edits are resolved in the same frame");
    SkBitmap firstPixels;firstPixels.allocN32Pixels(context->surface->width(),context->surface->height());context->surface->readPixels(firstPixels,0,0);
    expect(firstPixels.getColor(60,40)==SK_ColorGREEN,"the actual first framebuffer contains the laid-out probe's green paint");
    const auto initial=probe->painted;renderApplicationFrame();expect(probe->painted==initial,"second frame does not repair startup paint geometry");
    SkBitmap nextPixels;nextPixels.allocN32Pixels(context->surface->width(),context->surface->height());context->surface->readPixels(nextPixels,0,0);
    expect(std::memcmp(firstPixels.getPixels(),nextPixels.getPixels(),firstPixels.computeByteSize())==0,"first and second presented framebuffers are pixel-identical without an intervening mutation");
    auto animation=new_shared<CABasicAnimation>("opacity");animation->duration=30;animation->fromValue=1.f;animation->toValue=.9f;controller->host->layer()->add(animation,"opacity");
    for(int i=0;i<24;++i){
        context->size={320+i*7.25f,480-i*6.5f};context->safe={20-i*.5f,i*.125f,10+i*.25f,0};
        controller->inset=17+i*.5f;controller->view()->setNeedsLayout();
        renderApplicationFrame();
        expect(probe->bounds().size==controller->host->bounds().insetBy({-3,-5,-7,-11}).size,"Yoga resize relayouts manual descendants before presentation");
        expect(probe->painted==probe->bounds().size,"every interpolated resize paints current geometry without a repair frame");
        expect(controller->host->layer()->presentationOrSelf()->bounds().size==controller->host->bounds().size,"a layer with an unrelated animation presents current layout bounds in this frame");
        expect(near(controller->host->frame().minX(),controller->inset),"late callback edits are not one size sample behind");
    }
    for(float factor:{.75f,1.25f,1.f}){
        context->setExtraScaleFactor(factor);renderApplicationFrame();
        expect(window->bounds().size==context->size*factor,"the current UI scale reaches logical window bounds in the same frame");
        expect(probe->painted==probe->bounds().size&&controller->host->layer()->presentationOrSelf()->bounds().size==controller->host->bounds().size,"UI scale changes keep layout, paint and animated presentation geometry synchronized");
    }
    controller->host->layer()->removeAllAnimations();
    window->setRootViewController(nullptr);UIApplication::shared->keyWindow.reset();
}
void environmentAndMutation(const std::shared_ptr<Context>& context){
    context->size={320,480};context->safe={20,0,30,0};
    auto window=new_shared<UIWindow>();auto controller=new_shared<UIViewController>();
    auto root=new_shared<UIView>();auto child=new_shared<UIView>(NXRect(0,0,320,480));root->addSubview(child);controller->setView(root);
    window->setRootViewController(controller);window->makeKeyAndVisible();renderApplicationFrame();
    expect(child->safeAreaInsets()==UIEdgeInsets(20,0,30,0),"initial descendant safe area reflects the current window");
    context->size={320,600};renderApplicationFrame();
    expect(child->safeAreaInsets().bottom==0,"a parent's resize updates a fixed child's safe area even when that child's frame does not change");
    child->setInsetsLayoutMarginsFromSafeArea(false);child->setPreservesSuperviewLayoutMargins(true);root->setInsetsLayoutMarginsFromSafeArea(false);
    root->setLayoutMargins({0,12,0,8});root->layoutIfNeeded();expect(child->layoutMargins().left==root->layoutMargins().left&&child->layoutMargins().left>=12,"initial inherited layout margins are resolved synchronously");
    root->setLayoutMargins({0,24,0,16});root->layoutIfNeeded();expect(child->layoutMargins().left==24,"parent margin changes reach already clean descendants in the same pass");
    auto mutator=new_shared<LayoutAction>(),removed=new_shared<LayoutAction>(),added=new_shared<LayoutAction>();root->addSubview(mutator);root->addSubview(removed);
    mutator->action=[&]{removed->removeFromSuperview();root->addSubview(added);mutator->action=nullptr;};root->layoutIfNeeded();
    expect(removed->calls==0&&added->calls==1,"layout callback hierarchy edits use stable membership and lay out newly added children before returning");
    auto reentrant=new_shared<LayoutAction>();reentrant->action=[&]{reentrant->setNeedsLayout();reentrant->layoutIfNeeded();};
    reentrant->layoutIfNeeded();expect(reentrant->calls==8,"self-invalidating recursive callbacks are bounded instead of hanging or overflowing");
    reentrant->action=nullptr;reentrant->layoutIfNeeded();const auto count=reentrant->calls;reentrant->layoutIfNeeded();expect(count==9&&reentrant->calls==count,"bounded layout retains pending work and stops when it converges");
    root->addSubview(new_shared<PaintProbe>());root->subviews().back()->setHidden(true);root->drawAndLayoutTreeIfNeeded();
    auto hidden=std::static_pointer_cast<PaintProbe>(root->subviews().back());expect(hidden->draws==0&&hidden->layouts==0,"automatic frame preparation keeps hidden subtrees deferred");
    hidden->setHidden(false);root->drawAndLayoutTreeIfNeeded();expect(hidden->draws==1&&hidden->layouts>0,"revealed content is laid out before its first paint");
    window->setRootViewController(nullptr);UIApplication::shared->keyWindow.reset();
}
void animatedBitmap(const std::shared_ptr<Context>& context){
    auto window=new_shared<UIWindow>();auto controller=new_shared<UIViewController>();auto root=new_shared<UIView>();root->setBackgroundColor(UIColor::black);
    auto image=new_shared<BitmapProbe>();image->setFrame({0,0,100,40});root->addSubview(image);controller->setView(root);window->setRootViewController(controller);window->makeKeyAndVisible();renderApplicationFrame();
    UIView::animate(30,[&]{image->setAlpha(0);});
    image->setFrame({0,0,200,40});renderApplicationFrame();
    const auto presented=image->layer()->presentationOrSelf();
    expect(presented->contents()==image->layer()->contents()&&presented->contentsScale()==2,"animated views display regenerated bitmap contents and density without waiting for another frame");
    expect(image->alpha()==0&&presented->opacity()>.9f&&image->laidOut==image->bounds().size,"a still-visible fading-out view participates in the current layout pass");
    SkBitmap pixels;pixels.allocN32Pixels(context->surface->width(),context->surface->height());context->surface->readPixels(pixels,0,0);
    expect(SkColorGetG(pixels.getColor(40,40))>240&&SkColorGetR(pixels.getColor(40,40))<10,"the resize framebuffer contains the new green bitmap rather than its red predecessor");
    image->layer()->removeAllAnimations();window->setRootViewController(nullptr);UIApplication::shared->keyWindow.reset();
    auto density=new_shared<CALayer>();density->setContentsScale(1);
    auto change=new_shared<CABasicAnimation>("contentsScale");change->duration=1;change->fromValue=1.f;change->toValue=3.f;density->add(change,"contentsScale");density->animateAt(Timer(500));
    const auto intermediate=density->presentationOrSelf()->contentsScale();density->setContentsScale(4);
    expect(intermediate>1&&intermediate<3&&density->presentationOrSelf()->contentsScale()==intermediate,"live bitmap density updates preserve an explicitly animated presentation scale");density->removeAllAnimations();
    auto pulse=new_shared<CABasicAnimation>("opacity");pulse->duration=1;pulse->delay=.1;pulse->fromValue=1.f;pulse->toValue=.5f;
    pulse->timingFunction=new_shared<CAMediaTimingFunction>([](double x){if(x>=.49&&x<=.51)return 1.;return x<.5?x*2:(1-x)*2;});density->add(pulse,"opacity");
    auto at=[&](double ms){auto time=pulse->creationTime;const auto us=time.startTime.tv_usec+long(ms*1000);time.startTime.tv_sec+=us/1000000;time.startTime.tv_usec=us%1000000;density->animateAt(time);};
    at(600);expect(density->presentation()&&std::abs(density->presentation()->opacity()-.5f)<.00001f,"a timing curve crossing one does not finish before its duration");
    at(900);expect(density->presentation()&&density->presentation()->opacity()>.5f,"a pulse continues through its return phase");
    at(1101);expect(!density->presentation()&&density->opacity()==1,"a pulse returning to zero completes by elapsed time and restores its model value");
}
void explicitSubtree(){
    auto root=new_shared<UIView>(NXRect(0,0,200,100));root->setAutolayoutEnabled(true);
    auto host=new_shared<ManualHost>();host->setAutolayoutEnabled(true);host->configureLayout([](const auto& y){y->setFlexGrow(1);});root->addSubview(host);
    root->layoutIfNeeded();
    expect(host->child->bounds().size==NXSize(184,90),"layoutIfNeeded resolves the complete dirty subtree");
    root->setFrame({0,0,260,150});root->layoutIfNeeded();
    expect(host->child->bounds().size==NXSize(244,140),"Yoga-driven bounds invalidate a previously clean manual subtree");
}
void displayScale(){
    auto root=new_shared<FractionalRoot>();root->setFrame({0,0,200,100});root->setAutolayoutEnabled(true);
    auto child=new_shared<UIView>();child->setAutolayoutEnabled(true);child->configureLayout([](const auto& y){y->setSize({10.25f,10.25f});y->setAlignSelf(YGAlignFlexStart);});root->addSubview(child);
    for(float scale:{1.f,2.f,3.f,2.f}){root->scale(scale);root->layoutIfNeeded();expect(near(child->bounds().width(),std::ceil(10.25f*scale)/scale),"Yoga measured-node rounding follows the current display scale, including a return to 2x");}
}
}
int main(){
    expect(NXRect(0,0,10,20)!=NXRect(0,0,11,20)&&!(NXRect(0,0,10,20)!=NXRect(0,0,10,20)),"rectangle inequality detects size changes instead of equality");
    auto context=std::make_shared<Context>();SkiaCtx::_main=context;UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
    explicitSubtree();displayScale();firstPaintAndResize(context);environmentAndMutation(context);animatedBitmap(context);
    UIApplication::shared.reset();SkiaCtx::_main.reset();
    if(!failures)std::cout<<"Layout pass tests passed\n";return failures?1:0;
}
