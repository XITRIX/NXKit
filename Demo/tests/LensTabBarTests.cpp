#include "../../Demo/app/Screens/GlassDemoViewController/LensTabBarView.hpp"
#include "../../Demo/app/Screens/GlassDemoViewController/TabSelectionMaterial.hpp"
#include <cmath>
#include <iostream>
#include <fstream>
#include <limits>
#include <UIEvent.h>
#include <UITouch.h>
#include "CABackdropEffectLayer.hpp"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkBitmap.h"
using namespace NXKit;
namespace NXKit {
class GlassMaterialTestHarness {
public:
    static auto traits(bool dark) {auto value=std::make_shared<UITraitCollection>();value->_displayScale=2;value->_userInterfaceStyle=dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;return value;}
};
class RestingTintRoot final:public UIView {
public:
    RestingTintRoot(){_traitCollection=GlassMaterialTestHarness::traits(false);setFrame({0,0,200,100});}
    void darken(){auto previous=_traitCollection;_traitCollection=GlassMaterialTestHarness::traits(true);traitCollectionDidChange(previous);}
};
class LensTabTestWindow final : public UIWindow {
public:
    void layoutSubviews() override { UIView::layoutSubviews(); }
};
class UIGestureRecognizerTestHarness {
public:
    static auto event(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<UITouch>& touch) {
        auto event=new_shared<UIEvent>(); event->_allTouches.insert(touch); touch->_window=window; return event;
    }
    static void send(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<UIEvent>& event,
                     const std::shared_ptr<UITouch>& touch,UITouchPhase phase,NXPoint point,double milliseconds) {
        touch->updateAbsoluteLocation(point); touch->_phase=phase; touch->_timestamp=Timer(milliseconds); window->sendEvent(event);
    }
};
class GlassInteractionTestHarness {
public:
    static void advance(const std::shared_ptr<BackdropEffectView>& view,double seconds) { view->_advanceInteraction(seconds); }
    static auto motion(const std::shared_ptr<BackdropEffectView>& view) { return *view->_backdropEffectLayer()->_interaction; }
};
class LensViewTestHarness {
public:
    static void advance(const std::shared_ptr<LensView>& view,double seconds) { view->advance(seconds); }
    static NXFloat liftProgress(const std::shared_ptr<LensView>& view) { return view->_progress; }
    static void elastic(const std::shared_ptr<LensView>& view,NXAffineTransform value) { view->applyInteractionTransform(value); }
};
}
class LensTabBarTestHarness {
public:
    static void advance(const std::shared_ptr<LensTabBarView>& bar,double seconds) { bar->advanceLift(seconds); }
    static bool hasPulse(const std::shared_ptr<LensTabBarView>& bar) { return bar->hasPulse(); }
};
int main(int argc,char** argv) {
    int failures=0; auto expect=[&](bool pass,const char* message) { if (!pass) { ++failures; std::cerr<<"FAIL: "<<message<<'\n'; } };
    // Independent native resting-fill probes. Trait changes must replace
    // the copied transfer without relying on a touch or selection mutation.
    auto tintRoot=new_shared<RestingTintRoot>();tintRoot->setBackgroundColor(UIColor(195,195,195));
    auto tint=new_shared<TabSelectionBackground>();tint->setFrame({20,20,120,64});tintRoot->addSubview(tint);
    const auto sampleTint=[&] {
        auto target=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(200,100));target->getCanvas()->clear(SK_ColorTRANSPARENT);tintRoot->layer()->skiaRender(target->getCanvas());SkBitmap image;image.allocN32Pixels(200,100);target->readPixels(image,0,0);return image.getColor(80,50);
    };
    auto actualTint=sampleTint();expect(std::abs(int(SkColorGetR(actualTint))-169)<=2,"resting light selection matches native material transfer instead of an alpha overlay");
    tintRoot->darken();tintRoot->setBackgroundColor(UIColor(133,134,133));actualTint=sampleTint();
    expect(std::abs(int(SkColorGetR(actualTint))-99)<=2&&std::abs(int(SkColorGetB(actualTint))-98)<=2,"resting selection switches to the measured dark transfer when traits change");
    tint->setAlpha(.5f);actualTint=sampleTint();expect(std::abs(int(SkColorGetR(actualTint))-116)<=2,"resting selection fades by coverage when the lens lifts");
    auto bar=new_shared<LensTabBarView>(); bar->setFrame({0,0,640,76}); bar->layoutIfNeeded();
    expect(bar->glass()->glassEffect()->isInteractive()&&bar->lens()->isInteractive(),"platter and selection both opt into their own interactive response");
    auto source=bar->lens()->liftedContentView();
    expect(source&&source->superview().lock()!=bar->lens()->contentView(),"selected row is a separate underlying source, not content placed inside lens");
    const auto sourceFrame=source?source->frame():NXRect();
    expect(bar->lens()->isContentVibrancyEnabled(),"the default semantic tab accent opts into native-style foreground blending");
    bar->setAccentColor(UIColor(0,136,255));expect(!bar->lens()->isContentVibrancyEnabled(),"a literal accent keeps its RGB source-over behavior even when it matches a system color");
    bar->setAccentColor(UIColor::systemRed);expect(bar->lens()->isContentVibrancyEnabled(),"non-blue semantic system accents also use the foreground blend path");
    bar->setAccentColor(UIColor::systemBlue);
    int callbacks=0; bar->onSelectionChanged=[&](size_t index) { ++callbacks; expect(index==bar->selectedIndex(),"callback observes committed tab selection"); };
    expect(bar->selectedIndex()==0&&!bar->isLifted(),"tab starts at first resting selection");
    bar->controls()[2]->performPrimaryAction();
    expect(bar->selectedIndex()==2&&callbacks==1,"normal control action changes selection once");
    bar->setSelectedIndex(2); expect(callbacks==1,"reselecting current tab does not repeat callback");
    const auto resting=bar->lens()->frame(); bar->setLifted(true); const auto raised=bar->lens()->frame();
    expect(raised.width()==resting.width()+16&&raised.height()==resting.height()+16,"tab lift adds eight points per edge as observed on the native control");
    expect(bar->lens()->isLifted()&&bar->glass()->glassEffect()->style()==UIGlassEffect::Style::regular,"lens is separate from regular glass platter");
    bar->setSelectionPosition(.5f); expect(bar->selectedIndex()==2&&bar->selectionPosition()==.5f,"drag preview does not prematurely commit selected tab");
    expect(source&&source->frame()==sourceFrame,"dragging moves the lens across a stationary full selected row");
    bar->setSelectionPosition(std::numeric_limits<float>::quiet_NaN()); expect(bar->selectionPosition()==.5f,"nonfinite drag input cannot poison geometry");
    bar->setSelectionPosition(-100); expect(bar->selectionPosition()==0,"drag preview clamps to first tab");
    bar->setSelectionPosition(100); expect(bar->selectionPosition()==2,"drag preview clamps to last tab");
    bool rejected=false; try { bar->setSelectedIndex(3); } catch (const std::out_of_range&) { rejected=true; }
    expect(rejected&&bar->selectedIndex()==2,"invalid tab index preserves previous selection");
    bar->setLifted(false); expect(!bar->lens()->isLifted()&&bar->lens()->frame()==resting,"release restores the same selected pill geometry");
    bar->lens()->layer()->removeAllAnimations();
    auto normalMask=bar->glass()->subviews().front()->mask();
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(640,76));
    surface->getCanvas()->clear(SK_ColorTRANSPARENT); normalMask->layer()->draw(surface->getCanvas());
    SkBitmap pixels; pixels.allocN32Pixels(640,76); surface->readPixels(pixels,0,0);
    expect(SkColorGetA(pixels.getColor(533,38))==0&&SkColorGetA(pixels.getColor(106,38))==255,"ordinary row is punched out only beneath the selected lens");
    bar->setSelectionPosition(0); surface->getCanvas()->clear(SK_ColorTRANSPARENT); normalMask->layer()->draw(surface->getCanvas()); surface->readPixels(pixels,0,0);
    expect(SkColorGetA(pixels.getColor(106,38))==0&&SkColorGetA(pixels.getColor(533,38))==255,"punchout follows selection without removing other tab content");
    auto compact=new_shared<LensTabBarView>(true); compact->setFrame({0,0,293.5f,44}); compact->layoutIfNeeded();
    expect(compact->lens()->frame()==NXRect(4,4,84,36),"compact Home geometry matches observed native tab selection");
    compact->setSelectedIndex(1); expect(compact->lens()->frame()==NXRect(92,4,99.5f,36),"compact Favorites geometry follows content width");
    compact->setSelectedIndex(2); expect(compact->lens()->frame()==NXRect(195.5f,4,94,36),"compact Settings geometry matches observed native selection");
    compact->setFrame({0,0,220,44}); compact->layoutIfNeeded(); expect(std::isfinite(compact->lens()->frame().width())&&compact->lens()->frame().maxX()<=220,"resize retains selected slot inside platter");

    auto four=new_shared<LensTabBarView>(false,4); four->setFrame({0,0,360,62}); four->layoutIfNeeded();
    expect(four->controls().size()==4&&four->lens()->frame()==NXRect(4,4,95,54),"four-tab phone platter matches observed native first selection");
    four->setLifted(true);expect(four->lens()->frame().size==NXKit::NXSize(111,70)&&four->lens()->frame().minY()==-4,"lifted phone lens matches native111x70 bounds and extends beyond the platter");four->setLifted(false);
    const auto selectedItems=four->lens()->liftedContentView()->subviews();
    expect(selectedItems.size()==4,"each selected-source tab owns a separate scaling pivot");
    four->setLifted(true);
    for(size_t i=0;i<selectedItems.size();++i) {
        expect(std::abs(selectedItems[i]->transform().m11-1.16f)<.0001f&&std::abs(selectedItems[i]->transform().m22-1.16f)<.0001f,"all selected-source items enlarge sixteen percent while held");
        const auto pivot=NXPoint(51.5f+i*85.6666667f,31);
        const auto transformed=pivot.applying(selectedItems[i]->transform());
        expect(std::abs(transformed.x-pivot.x)<.001f&&std::abs(transformed.y-pivot.y)<.001f,"selected ink scales about each native item center");
    }
    four->setSelectionPosition(1.5f);
    four->setFrame({0,0,400,62});four->layoutIfNeeded();
    expect(std::abs(selectedItems[0]->transform().m11-1.16f)<.0001f,"resize and between-item dragging retain lifted source enlargement");
    four->setLifted(false);
    for(const auto& item:selectedItems)expect(item->transform().isIdentity(),"release returns every selected-source item to its resting scale");
    four->setFrame({0,0,360,62});four->layoutIfNeeded();
    four->setLifted(true,true);
    const auto advanceItems=[&](const auto& self,const std::shared_ptr<CALayer>& layer,Timer time)->void {
        layer->animateAt(time);for(const auto& child:layer->sublayers())self(self,child,time);
    };
    advanceItems(advanceItems,four->layer(),Timer(10));
    advanceItems(advanceItems,four->layer(),Timer(100));
    const auto intermediate=selectedItems[2]->layer()->presentationOrSelf()->affineTransform().m11;
    expect(intermediate>1&&intermediate<1.16f,"lift animates selected-source magnification without jumping to the final scale");
    four->setLifted(false,true);advanceItems(advanceItems,four->layer(),Timer(2000));
    for(const auto& item:selectedItems)expect(item->layer()->presentationOrSelf()->affineTransform().isIdentity(),"interrupted lift settles to exact resting source geometry");

    four->setSelectedIndex(2); expect(std::abs(four->lens()->frame().minX()-175.333333f)<.001f,"four-tab selected lens follows the measured item centers");
    four->setSelectedIndex(3); expect(four->lens()->frame().maxX()<=356.001f,"fourth selection retains the trailing four-point inset");
    four->setSelectionPosition(2.5f); expect(four->selectedIndex()==3&&std::isfinite(four->lens()->frame().minX()),"four-tab drag preview interpolates without committing an action");

    // Exercise real touch delivery through descendant controls, the selection
    // pan, and both passive material observers rather than calling callbacks.
    auto window=new_shared<LensTabTestWindow>(); window->setFrame({0,0,800,400});
    auto interactive=new_shared<LensTabBarView>(); interactive->setFrame({80,120,640,76}); window->addSubview(interactive); interactive->layoutIfNeeded();
    const auto tickLayers=[&](const auto& self,const std::shared_ptr<CALayer>& layer,Timer now)->void {
        layer->animateAt(now);
        for(const auto& child:layer->sublayers()) self(self,child,now);
    };
    const auto settleMaterials=[&] {
        for(int i=1;i<=360;++i) {
            tickLayers(tickLayers,interactive->layer(),Timer(i*1000.0/120));
            LensTabBarTestHarness::advance(interactive,1.0/120);
            GlassInteractionTestHarness::advance(interactive->glass(),1.0/120);
            LensViewTestHarness::advance(interactive->lens(),1.0/120);
        }
    };
    int touchSelections=0; interactive->onSelectionChanged=[&](size_t) { ++touchSelections; };
    const auto tap=[&](unsigned long id,NXPoint point) {
        auto touch=new_shared<UITouch>(id,point,Timer()); auto event=UIGestureRecognizerTestHarness::event(window,touch);
        UIGestureRecognizerTestHarness::send(window,event,touch,UITouchPhase::began,point,0);
        expect(std::abs(interactive->lens()->frame().midX()-(point.x-80))<1&&interactive->isLifted(),"touch-down outside the lens immediately targets the finger and lifts without waiting for a pan");
        GlassInteractionTestHarness::advance(interactive->glass(),.15);
        expect(GlassInteractionTestHarness::motion(interactive->glass()).glow()>.9f,"a tab contact lights the interactive platter through its descendant control");
        expect(!touch->hasBeenCancelledByAGestureRecognizer(),"passive lens and platter feedback leave a stationary tab tap uncancelled");
        UIGestureRecognizerTestHarness::send(window,event,touch,UITouchPhase::ended,point,150);
        settleMaterials();
    };
    tap(4001,{80+533,158});
    expect(interactive->selectedIndex()==2&&touchSelections==1,"a real tap commits exactly one selection with both materials interactive");
    tap(4002,{80+533,158});
    expect(interactive->selectedIndex()==2&&touchSelections==1,"a failed pan during a tap cannot restore an old drag selection or emit extra actions");
    interactive->setSelectedIndex(0); touchSelections=0;
    tickLayers(tickLayers,interactive->layer(),Timer(1000));
    auto remote=new_shared<UITouch>(4010,NXPoint(613,158),Timer()); auto remoteEvent=UIGestureRecognizerTestHarness::event(window,remote);
    UIGestureRecognizerTestHarness::send(window,remoteEvent,remote,UITouchPhase::began,{613,158},0);
    expect(interactive->selectionPosition()>1.99f&&interactive->selectedIndex()==0&&touchSelections==0,"remote touch-down previews the finger without committing selection");
    expect(interactive->lens()->layer()->presentation()&&interactive->lens()->layer()->presentation()->position().x<200,"remote approach animates from the presented lens instead of teleporting");
    UIGestureRecognizerTestHarness::send(window,remoteEvent,remote,UITouchPhase::moved,{608,158},15);
    expect(interactive->isLifted()&&interactive->lens()->isLifted(),"small finger movement retains lift before the pan recognizes");
    UIGestureRecognizerTestHarness::send(window,remoteEvent,remote,UITouchPhase::moved,{590,158},30);
    expect(interactive->isLifted()&&interactive->lens()->isLifted(),"pan recognition cannot lower the lens when it cancels the tab's highlight");
    UIGestureRecognizerTestHarness::send(window,remoteEvent,remote,UITouchPhase::moved,{480,158},80);
    expect(std::abs(interactive->lens()->frame().midX()-400)<.01f,"drag uses the absolute finger position rather than retaining its touch-down offset");
    UIGestureRecognizerTestHarness::send(window,remoteEvent,remote,UITouchPhase::cancelled,{480,158},100);
    settleMaterials();
    expect(interactive->selectedIndex()==0&&interactive->selectionPosition()==0&&touchSelections==0,"cancelled remote preview returns to its original committed selection");
    interactive->setSelectedIndex(1);settleMaterials();touchSelections=0;
    const auto grabbedCenter=interactive->lens()->frame().midX();
    const auto grabbedStart=NXPoint(80+grabbedCenter+30,158);
    auto grabbed=new_shared<UITouch>(4011,grabbedStart,Timer());auto grabbedEvent=UIGestureRecognizerTestHarness::event(window,grabbed);
    UIGestureRecognizerTestHarness::send(window,grabbedEvent,grabbed,UITouchPhase::began,grabbedStart,0);
    expect(std::abs(interactive->lens()->frame().midX()-grabbedCenter)<.01f,"touch-down inside the lens preserves its off-center grab instead of recentering it");
    UIGestureRecognizerTestHarness::send(window,grabbedEvent,grabbed,UITouchPhase::moved,grabbedStart+NXPoint(5,0),15);
    expect(std::abs(interactive->lens()->frame().midX()-grabbedCenter-5)<.01f,"a grabbed lens follows small movement before the pan threshold");
    GlassInteractionTestHarness::advance(interactive->glass(),.15);
    expect(!interactive->glass()->interactionTransform().isIdentity(),"held glass exposes its visual inflation for contact alignment");
    UIGestureRecognizerTestHarness::send(window,grabbedEvent,grabbed,UITouchPhase::moved,grabbedStart+NXPoint(80,0),100);
    const auto visibleGrabbedCenter=NXPoint(interactive->lens()->frame().midX(),38).applying(interactive->glass()->interactionTransform());
    expect(std::abs(visibleGrabbedCenter.x-(grabbedStart.x-80+80-30))<.01f&&interactive->isLifted(),"drag preserves the finger offset as the containing glass inflates");
    UIGestureRecognizerTestHarness::send(window,grabbedEvent,grabbed,UITouchPhase::cancelled,grabbedStart+NXPoint(80,0),120);settleMaterials();
    expect(interactive->selectedIndex()==1&&touchSelections==0&&!interactive->isLifted(),"cancelling an off-center grab restores selection and lift without an action");
    const auto cornerFrame=interactive->lens()->frame();const auto corner=NXPoint(80+cornerFrame.minX()+2,120+cornerFrame.minY()+2);
    auto cornerTouch=new_shared<UITouch>(4012,corner,Timer());auto cornerEvent=UIGestureRecognizerTestHarness::event(window,cornerTouch);
    UIGestureRecognizerTestHarness::send(window,cornerEvent,cornerTouch,UITouchPhase::began,corner,0);
    expect(std::abs(interactive->lens()->frame().midX()-(corner.x-80))<.01f,"touch in a lens bounding-box corner outside its capsule uses the animated approach");
    UIGestureRecognizerTestHarness::send(window,cornerEvent,cornerTouch,UITouchPhase::cancelled,corner,30);settleMaterials();
    interactive->setSelectedIndex(1); touchSelections=0;
    tickLayers(tickLayers,interactive->layer(),Timer(1000));
    const auto platterFrame=interactive->glass()->frame(), fixedSourceFrame=interactive->lens()->liftedContentView()->frame();
    auto touch=new_shared<UITouch>(4003,NXPoint(400,158),Timer()); auto event=UIGestureRecognizerTestHarness::event(window,touch);
    UIGestureRecognizerTestHarness::send(window,event,touch,UITouchPhase::began,{400,158},0);
    UIGestureRecognizerTestHarness::send(window,event,touch,UITouchPhase::moved,{420,158},30);
    tickLayers(tickLayers,interactive->layer(),Timer(30)); LensViewTestHarness::advance(interactive->lens(),.03);
    UIGestureRecognizerTestHarness::send(window,event,touch,UITouchPhase::moved,{535,174},80);
    tickLayers(tickLayers,interactive->layer(),Timer(80)); LensViewTestHarness::advance(interactive->lens(),.05);
    GlassInteractionTestHarness::advance(interactive->glass(),.12); LensViewTestHarness::advance(interactive->lens(),.12);
    expect(interactive->selectionPosition()>1&&interactive->selectedIndex()==1&&touchSelections==0,"drag preview runs while the interactive platter observes the same contact");
    expect(!interactive->lens()->interactionTransform().isIdentity(),"a routed tab drag produces visual lens deformation");
    expect(interactive->glass()->frame()==platterFrame&&interactive->lens()->liftedContentView()->frame()==fixedSourceFrame,"interactive feedback preserves platter layout and the stationary selected-source row");
    UIGestureRecognizerTestHarness::send(window,event,touch,UITouchPhase::ended,{535,174},100);
    settleMaterials();
    expect(interactive->selectedIndex()==2&&touchSelections==1,"drag release commits one tab action without a competing glass recognizer");
    expect(GlassInteractionTestHarness::motion(interactive->glass()).atRest()&&interactive->lens()->interactionTransform().isIdentity(),"both materials settle after a routed drag release");
    tap(4004,{80+533,158});
    expect(touchSelections==1,"tapping after a completed drag never reselects the drag's old tab");
    auto cancelled=new_shared<UITouch>(4005,NXPoint(613,158),Timer()); auto cancelledEvent=UIGestureRecognizerTestHarness::event(window,cancelled);
    UIGestureRecognizerTestHarness::send(window,cancelledEvent,cancelled,UITouchPhase::began,{613,158},0);
    UIGestureRecognizerTestHarness::send(window,cancelledEvent,cancelled,UITouchPhase::moved,{590,158},30);
    UIGestureRecognizerTestHarness::send(window,cancelledEvent,cancelled,UITouchPhase::moved,{480,158},80);
    UIGestureRecognizerTestHarness::send(window,cancelledEvent,cancelled,UITouchPhase::cancelled,{480,158},100);
    settleMaterials();
    expect(interactive->selectedIndex()==2&&touchSelections==1&&interactive->lens()->interactionTransform().isIdentity(),"cancelling a drag restores its committed selection and settles without an action");
    for(const auto& geometryBar:{compact,four}) {
        geometryBar->setFrame({80,230,geometryBar==compact?293.5f:360.0f,geometryBar==compact?44.0f:62.0f});
        window->addSubview(geometryBar); geometryBar->layoutIfNeeded();
        geometryBar->setSelectedIndex(1);
        const auto targetCenter=geometryBar->lens()->frame().midX();
        geometryBar->setSelectedIndex(0);
        const auto start=NXPoint(80+targetCenter,geometryBar->frame().midY());
        auto contact=new_shared<UITouch>(geometryBar==compact?4020:4021,start,Timer());
        auto input=UIGestureRecognizerTestHarness::event(window,contact);
        UIGestureRecognizerTestHarness::send(window,input,contact,UITouchPhase::began,start,0);
        expect(std::abs(geometryBar->lens()->frame().midX()-targetCenter)<.01f&&geometryBar->selectedIndex()==0,"remote touch-down targets actual item centers in compact and four-tab layouts");
        UIGestureRecognizerTestHarness::send(window,input,contact,UITouchPhase::moved,start+NXPoint(20,0),30);
        expect(std::abs(geometryBar->lens()->frame().midX()-targetCenter-20)<.01f,"compact unequal-width and four-tab lenses track finger coordinates exactly");
        expect(geometryBar->isLifted()&&geometryBar->lens()->isLifted(),"dragged compact and phone lenses retain their lifted state");
        for(int i=0;i<120;++i) LensViewTestHarness::advance(geometryBar->lens(),1.0/120);
        expect(LensViewTestHarness::liftProgress(geometryBar->lens())>.99f,"pausing a drag keeps the refractive lens fully lifted");
        UIGestureRecognizerTestHarness::send(window,input,contact,UITouchPhase::cancelled,start+NXPoint(20,0),50);
        expect(geometryBar->selectedIndex()==0&&geometryBar->selectionPosition()==0,"geometry-specific drag cancellation preserves the original tab");
        expect(!geometryBar->isLifted()&&!geometryBar->lens()->isLifted(),"cancellation lowers the lens after ending its contact");
        geometryBar->removeFromSuperview();
    }
    // Actual short taps: the selection action commits on release while the
    // presentation lens remains enlarged above the platter during transit.
    // Native first/fourth, reverse and adjacent captures are retained in
    // docs/liquid-glass-lens/tab-transfer (95x54 resting, 111x70 lifted).
    auto transfer=new_shared<LensTabBarView>(false,4);transfer->setFrame({80,260,360,62});
    window->addSubview(transfer);transfer->layoutIfNeeded();
    int transferActions=0;transfer->onSelectionChanged=[&](size_t){++transferActions;};
    std::ofstream transferTrace;
    const auto stepTransfer=[&](Timer origin,int tick) {
        origin.startTime.tv_usec+=int(tick*1000000.0/120);
        tickLayers(tickLayers,transfer->layer(),origin);
        LensTabBarTestHarness::advance(transfer,1.0/120);
        LensViewTestHarness::advance(transfer->lens(),1.0/120);
        GlassInteractionTestHarness::advance(transfer->glass(),1.0/120);
        if(transferTrace.is_open()) {
            const auto layer=transfer->lens()->layer()->presentationOrSelf();
            const auto elastic=transfer->lens()->interactionTransform();
            transferTrace<<tick/120.0<<','<<layer->position().x<<','<<layer->bounds().width()<<','<<layer->bounds().height()<<','<<transfer->isLifted()<<','<<LensViewTestHarness::liftProgress(transfer->lens())<<','<<elastic.m11<<','<<elastic.m22<<'\n';
        }
    };
    for(const auto indices:{std::pair<size_t,size_t>{0,3},{3,0},{0,1},{1,1}}) {
        if(argc==2) {
            transferTrace.open(std::string(argv[1])+"/"+std::to_string(indices.first)+"-"+std::to_string(indices.second)+".csv");
            transferTrace<<"t,centerX,width,height,lifted,materialProgress,elasticX,elasticY\n";
        }
        transfer->setSelectedIndex(indices.first);transfer->lens()->layer()->removeAllAnimations();
        const auto startX=transfer->lens()->layer()->position().x;
        const auto point=NXPoint(80+51.5f+indices.second*(257.f/3),291);
        auto contact=new_shared<UITouch>(4100+indices.second,point,Timer());auto input=UIGestureRecognizerTestHarness::event(window,contact);
        const Timer origin;
        const auto priorActions=transferActions;
        UIGestureRecognizerTestHarness::send(window,input,contact,UITouchPhase::began,point,0);
        for(int i=1;i<=8;++i)stepTransfer(origin,i);
        const auto beforeRelease=transfer->lens()->layer()->presentationOrSelf()->position().x;
        UIGestureRecognizerTestHarness::send(window,input,contact,UITouchPhase::ended,point,1000.0/15);
        expect(transfer->selectedIndex()==indices.second&&transferActions-priorActions==(indices.first!=indices.second),"short tap commits exactly once at release, including reselect");
        expect(transfer->isLifted()&&transfer->lens()->bounds().size==NXSize(111,70),"short tap release retains the elevated overhanging lens");
        expect(std::abs(transfer->lens()->layer()->presentationOrSelf()->position().x-beforeRelease)<.001f,"tap action does not restart or jump the touch-down approach");
        for(int i=9;i<=24;++i)stepTransfer(origin,i);
        const auto presented=transfer->lens()->layer()->presentationOrSelf();
        expect(presented->bounds().height()>69&&presented->bounds().minY()==0,"lens remains enlarged through the middle of its flight");
        if(indices.first!=indices.second) {
            const auto arrived=std::abs(presented->position().x-transfer->lens()->layer()->position().x);
            expect(arrived>8&&std::abs(presented->position().x-startX)>20&&transfer->isLifted(),"travelling lens stays raised until near its destination");
            expect(!transfer->lens()->interactionTransform().isIdentity(),"tap travel drives the same elastic deformation as dragging");
        }
        for(int i=25;i<=240;++i)stepTransfer(origin,i);
        expect(!transfer->isLifted()&&!LensTabBarTestHarness::hasPulse(transfer)&&transfer->lens()->bounds().size==NXSize(95,54),"arrival lowers to the exact resting size and releases the pulse display link");
        expect(transfer->lens()->interactionTransform().isIdentity(),"selection travel finishes without residual elastic transform");
        if(transferTrace.is_open()) transferTrace.close();
    }
    // Catch and reverse a travelling lens, then interrupt its next flight with
    // an immediate selection and detach another flight from the hierarchy.
    transfer->setSelectedIndex(0);transfer->setSelectedIndex(3,true);
    const Timer reversalOrigin;
    for(int i=1;i<=12;++i)stepTransfer(reversalOrigin,i);
    const auto caughtX=transfer->lens()->layer()->presentationOrSelf()->position().x;
    transfer->setSelectedIndex(0,true);
    expect(std::abs(transfer->lens()->layer()->presentationOrSelf()->position().x-caughtX)<.001f,"rapid selection reversal begins at the visible in-flight position");
    const Timer reverseStart;
    for(int i=1;i<=240;++i)stepTransfer(reverseStart,i);
    expect(transfer->selectedIndex()==0&&!transfer->isLifted()&&!LensTabBarTestHarness::hasPulse(transfer),"rapid reversal settles at the newest selection without a stale lowering callback");
    transfer->setSelectedIndex(3,true);transfer->setSelectedIndex(1);
    expect(transfer->selectedIndex()==1&&!transfer->isLifted()&&!LensTabBarTestHarness::hasPulse(transfer),"nonanimated selection cancels an automatic lift pulse immediately");
    transfer->setSelectedIndex(0);transfer->setSelectedIndex(3,true);
    const Timer catchStart;
    for(int i=1;i<=12;++i)stepTransfer(catchStart,i);
    const auto visibleCatch=transfer->lens()->layer()->presentationOrSelf()->position().x;
    const auto catchPoint=NXPoint(80+visibleCatch,291);
    auto catchTouch=new_shared<UITouch>(4200,catchPoint,Timer());auto catchEvent=UIGestureRecognizerTestHarness::event(window,catchTouch);
    UIGestureRecognizerTestHarness::send(window,catchEvent,catchTouch,UITouchPhase::began,catchPoint,0);
    expect(std::abs(transfer->lens()->frame().midX()-visibleCatch)<1&&transfer->isLifted(),"a new finger catches the visible travelling lens without teleporting to its target");
    UIGestureRecognizerTestHarness::send(window,catchEvent,catchTouch,UITouchPhase::cancelled,catchPoint,20);
    expect(transfer->selectedIndex()==3&&!transfer->isLifted()&&!LensTabBarTestHarness::hasPulse(transfer),"cancelling an in-flight catch preserves the committed tab and clears its pulse");
    transfer->setSelectedIndex(0);
    const auto holdPoint=NXPoint(388.5f,291);
    auto heldTouch=new_shared<UITouch>(4201,holdPoint,Timer());auto heldEvent=UIGestureRecognizerTestHarness::event(window,heldTouch);
    const Timer holdStart;
    UIGestureRecognizerTestHarness::send(window,heldEvent,heldTouch,UITouchPhase::began,holdPoint,0);
    for(int i=1;i<=120;++i)stepTransfer(holdStart,i);
    expect(transfer->isLifted()&&LensViewTestHarness::liftProgress(transfer->lens())>.99f,"a long remote press remains fully elevated after arriving while the finger is held");
    UIGestureRecognizerTestHarness::send(window,heldEvent,heldTouch,UITouchPhase::ended,holdPoint,1000);
    expect(!transfer->isLifted()&&transfer->selectedIndex()==3&&!LensTabBarTestHarness::hasPulse(transfer),"a long held press lowers immediately at release after arrival without another minimum hold");
    transfer->setSelectedIndex(0);transfer->setSelectedIndex(3,true);
    expect(LensTabBarTestHarness::hasPulse(transfer),"detachment regression begins with an active flight pulse");
    transfer->removeFromSuperview();LensTabBarTestHarness::advance(transfer,.01);
    expect(!transfer->isLifted()&&!LensTabBarTestHarness::hasPulse(transfer),"detaching during flight releases the pulse and lifted state");
    // Compare the actual retained lens silhouette with its control-owned
    // punchout under extra affine motion. No expected path is recreated here.
    auto maskBar=new_shared<LensTabBarView>(); maskBar->setFrame({0,0,640,76}); maskBar->layoutIfNeeded();
    auto maskLens=maskBar->lens(); maskLens->setRestingBackgroundColor(UIColor::white);
    maskLens->setLiftedContentView(nullptr); // This aperture check needs no font/application context.
    auto actualMask=maskBar->glass()->subviews().front()->mask();
    auto silhouetteSurface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(640,76));
    SkBitmap silhouettePixels; silhouettePixels.allocN32Pixels(640,76);
    for(const auto affine : {NXAffineTransform(1.12f,.05f,.10f,.90f,-5,2),
                            NXAffineTransform(.94f,-.09f,.04f,1.08f,4,-1)}) {
        LensViewTestHarness::elastic(maskLens,affine);
        surface->getCanvas()->clear(SK_ColorTRANSPARENT); actualMask->layer()->draw(surface->getCanvas()); surface->readPixels(pixels,0,0);
        silhouetteSurface->getCanvas()->clear(SK_ColorTRANSPARENT); maskLens->layer()->skiaRender(silhouetteSurface->getCanvas()); silhouetteSurface->readPixels(silhouettePixels,0,0);
        int mismatches=0;
        for(int y=1;y<75;++y) for(int x=1;x<639;++x) {
            const auto silhouetteAlpha=SkColorGetA(silhouettePixels.getColor(x,y));
            const auto maskAlpha=SkColorGetA(pixels.getColor(x,y));
            bool stable=true;
            // The retained fill is antialiased through both fill and clip;
            // exclude its one-device-pixel boundary when checking alignment.
            for(int dy=-1;dy<=1;++dy) for(int dx=-1;dx<=1;++dx)
                stable=stable&&((SkColorGetA(silhouettePixels.getColor(x+dx,y+dy))>128)==(silhouetteAlpha>128));
            if(stable&&((silhouetteAlpha>128)==(maskAlpha>128))) ++mismatches;
        }
        expect(mismatches==0,"normal-row punchout tracks the actual stretched and sheared retained lens silhouette");
    }
    if (failures) return 1; std::cout<<"Lens tab composition tests passed\n";
}
