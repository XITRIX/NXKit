#include <UISwitch.h>
#include <LensView.h>
#include <UIImageView.h>
#include <UIWindow.h>
#include <UIScrollView.h>
#include <UITouch.h>
#include <UIEvent.h>
#include <UINib.h>
#include <UIKitRegistration.h>
#include "RasterTestContext.hpp"
#include "SwitchNativeMotionData.hpp"
#include <iostream>
#include <limits>
#include <algorithm>

using namespace NXKit;
namespace NXKit {
class GlassMaterialTestHarness {
public:
    static auto traits(bool dark,NXFloat scale) {
        auto traits=std::make_shared<UITraitCollection>();traits->_displayScale=scale;
        traits->_userInterfaceStyle=dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;
        return traits;
    }
};
class LensViewTestHarness {
public:
    static void advance(const std::shared_ptr<LensView>& lens,double dt){lens->advance(dt);}
};
class UISwitchTestHarness {
public:
    static auto lens(const std::shared_ptr<UISwitch>& s){return s->_lens;}
    static auto position(const std::shared_ptr<UISwitch>& s){return s->_position;}
    static void advance(const std::shared_ptr<UISwitch>& s,double dt){
        // Match the independent lens/control display links without wall-clock
        // sleeps. Subdivide to resolve a deferred alongside callback accurately.
        while(dt>1e-12){const auto step=std::min(dt,1./120);LensViewTestHarness::advance(s->_lens,step);s->advance(step);dt-=step;}
    }
    static void press(const std::shared_ptr<UISwitch>& s,bool pressed,bool animated=true){s->setPressed(pressed,animated);}
    static bool accepted(const std::shared_ptr<UISwitch>& s){return s->_acceptedContact;}
    static auto trackColor(const std::shared_ptr<UISwitch>& s){return s->_track->backgroundColor();}
};
class UIGestureRecognizerTestHarness {
public:
    static void send(const std::shared_ptr<UIWindow>& w,const std::shared_ptr<UITouch>& t,UITouchPhase phase,NXKit::NXPoint p) {
        t->_window=w;t->_phase=phase;t->updateAbsoluteLocation(p);
        auto e=new_shared<UIEvent>();e->_allTouches.insert(t);w->sendEvent(e);
    }
};
}
namespace {
int failures=0;
void expect(bool result,const char* message){if(!result){++failures;std::cerr<<"FAIL: "<<message<<'\n';}}
void properties() {
    int changes=0;
    auto s=new_shared<UISwitch>(NXRect(10,20,200,90),UIAction("Changed",[&]{++changes;}));
    expect(!s->isOn() && !s->onTintColor() && !s->thumbTintColor(),"switch default state and nullable appearance overrides");
    expect(!s->onImage() && !s->offImage(),"switch starts without legacy image overrides");
    expect(UISwitchTestHarness::lens(s)->warpsContentBelow() && !UISwitchTestHarness::lens(s)->restingShadowProperties(),"switch uses native backdrop policy and no copied slider resting shadow");
    expect(s->frame().origin==NXKit::NXPoint(10,20) && s->frame().size==NXKit::NXSize(63,28) && s->sizeThatFits({200,90})==NXKit::NXSize(61,28),"native initializer and fitting sizes preserve their observed two-point width difference");
    expect(s->preferredStyle()==UISwitchStyle::automatic && s->style()==UISwitchStyle::sliding,"automatic resolves to concrete sliding style");
    s->setPreferredStyle(UISwitchStyle::checkbox);expect(s->style()==UISwitchStyle::sliding,"checkbox request retains portable sliding fallback");
    bool rejected=false;try{s->setPreferredStyle(UISwitchStyle(99));}catch(const std::invalid_argument&){rejected=true;}
    expect(rejected,"invalid style rejected");
    s->setOn(true);s->setOn(false,true);s->setOn(false);expect(changes==0 && !s->isOn(),"programmatic animated and immediate state changes never send actions");
    s->setOn(true,true);expect(!UISwitchTestHarness::lens(s)->isLifted(),"native programmatic state animation does not lift the lens");s->setOn(false);
    s->setOnTintColor(UIColor::systemPink);s->setThumbTintColor(UIColor::red);
    expect(s->onTintColor()==UIColor::systemPink && s->thumbTintColor()==UIColor::red,"appearance overrides are retained");
    expect(UISwitchTestHarness::lens(s)->restingBackgroundColor()==UIColor::red,"thumb tint uses shared resting lens surface");
    s->setThumbTintColor(std::nullopt);expect(!s->thumbTintColor(),"nil restores default thumb tint");
    auto image=UIImage::systemImageNamed("checkmark");s->setOnImage(image);s->setOffImage(image);
    expect(s->onImage()==image && s->offImage()==image,"legacy images retain their native property semantics");
    s->setOnImage(nullptr);s->setOffImage(nullptr);
    s->setBounds({5,7,100,50});s->layoutSubviews();
    expect(s->bounds().size==NXKit::NXSize(100,50),"direct bounds assignment remains available");
    s->setFrame({5,7,150,60});expect(s->frame().size==NXKit::NXSize(63,28),"later frame assignment enforces native sliding-provider size");
    s->setBounds({0,0,150,60});std::shared_ptr<UIView> polymorphic=s;polymorphic->sizeToFit();
    expect(s->frame().size==NXKit::NXSize(63,28),"native sizeToFit enforcement works through UIView polymorphic dispatch");
    expect(s->sizeThatFits({1,1})==s->sizeThatFits({1000,1000}),"sizeThatFits is independent of requested constraint");
    rejected=false;try{s->setFrame({0,0,std::numeric_limits<float>::infinity(),28});}catch(const std::invalid_argument&){rejected=true;}
    expect(rejected,"nonfinite geometry rejected");
    registerUIKitViews();auto xml=UINib::fromString("<UISwitch on=\"true\" preferredStyle=\"sliding\" thumbTintColor=\"#ff0000\"/>")->instantiate<UISwitch>();
    expect(xml && xml->isOn() && xml->preferredStyle()==UISwitchStyle::sliding && xml->thumbTintColor() && xml->thumbTintColor()->raw()==UIColor::red.raw(),"explicit optional UIKit XML registration includes switch");
    expect(!s->applyXMLAttribute("on","invalid") && !s->applyXMLAttribute("preferredStyle","invalid"),"invalid switch XML attributes fail visibly");
    s->setEnabled(false);expect(!s->accessibilityActivate() && changes==0,"disabled activation is rejected");
    s->setEnabled(true);expect(s->accessibilityActivate() && s->isOn() && changes==1,"portable accessibility activation toggles and sends one valueChanged");
    s->sendActions(UIControlEvent::valueChanged);expect(s->isOn() && changes==2,"explicit dispatch does not mutate switch state");
    s->sendActions(UIControlEvent::primaryActionTriggered);expect(s->isOn(),"primary event dispatch also does not mutate switch state");
}
void touchTracking() {
    auto w=new_shared<UIWindow>();w->setFrame({0,0,300,500});
    auto scroll=new_shared<UIScrollView>();scroll->setFrame(w->bounds());scroll->setContentSize({300,900});scroll->setDelaysContentTouches(false);w->addSubview(scroll);
    auto s=new_shared<UISwitch>(NXRect(40,100,1,1));scroll->addSubview(s);s->layoutSubviews();
    int changed=0,cancelled=0,downs=0,ups=0;std::vector<std::string> events;
    s->addAction(UIAction("Value",[&]{++changed;events.push_back("value");}),UIControlEvent::valueChanged);
    s->addAction(UIAction("Cancel",[&]{++cancelled;}),UIControlEvent::touchCancel);
    s->addAction(UIAction("Down",[&]{++downs;}),UIControlEvent::touchDown);
    s->addAction(UIAction("Up",[&]{++ups;events.push_back("up");}),UIControlEvent::touchUpInside);
    unsigned id=0;std::shared_ptr<UITouch> touch;
    auto begin=[&](NXKit::NXPoint point){touch=new_shared<UITouch>(++id,NXKit::NXPoint(),Timer());UIGestureRecognizerTestHarness::send(w,touch,UITouchPhase::began,point);};
    auto move=[&](NXKit::NXPoint point){UIGestureRecognizerTestHarness::send(w,touch,UITouchPhase::moved,point);};
    auto end=[&](NXKit::NXPoint point){UIGestureRecognizerTestHarness::send(w,touch,UITouchPhase::ended,point);UISwitchTestHarness::advance(s,1);};
    begin({55,114});expect(!s->isTracking() && !s->isHighlighted() && UISwitchTestHarness::lens(s)->isLifted(),"native switch recognizer lifts without entering UIControl tracking/highlight states");
    end({55,114});expect(s->isOn() && changed==1 && !s->isTracking(),"stationary tap toggles once");
    expect(downs==0 && ups==1 && events==std::vector<std::string>({"value","up"}),"native tap sends valueChanged before touchUpInside without touchDown");
    begin({85,114});end({85,114});expect(!s->isOn() && changed==2,"second stationary tap toggles off");
    begin({55,114});move({85,114});expect(!s->isOn() && changed==2,"drag preview does not prematurely change public on state");
    expect(scroll->contentOffset()==NXKit::NXPoint::zero,"drag owns touch without scrolling parent");
    end({85,114});expect(s->isOn() && changed==3,"drag past midpoint commits on once");
    expect(ups==2 && downs==0,"native drag emits no terminal touchUpInside or touchDown");
    begin({85,114});move({55,114});end({55,114});expect(!s->isOn() && changed==4,"drag back commits off once");
    begin({55,114});move({85,114});s->setEnabled(false);UISwitchTestHarness::advance(s,1);
    expect(!s->isTracking() && !s->isOn() && changed==4 && cancelled==1,"disable cancels preview without value change");
    expect(!UISwitchTestHarness::lens(s)->isLifted(),"cancel clears lens lift");s->setEnabled(true);
    begin({55,114});move({85,114});move({55,114});end({55,114});expect(!s->isOn() && changed==4,"drag returning to original side sends no change");
    begin({55,114});end({55,180});expect(!s->isOn() && changed==4,"un-dragged release outside leaves state unchanged");
    s->setOn(true,true);UISwitchTestHarness::advance(s,.08);const auto position=UISwitchTestHarness::position(s);
    s->setOn(false,true);expect(UISwitchTestHarness::position(s)==position,"retarget does not jump visible position");
    UISwitchTestHarness::advance(s,1);expect(UISwitchTestHarness::position(s)==0 && !UISwitchTestHarness::lens(s)->isLifted(),"programmatic return settles position and shared lens");
    begin({55,114});move({85,114});s->removeFromSuperview();UISwitchTestHarness::advance(s,1);
    expect(!s->isOn() && changed==4 && cancelled==2 && !UISwitchTestHarness::lens(s)->isLifted(),"removal cancels active preview without committing state");
    scroll->addSubview(s);s->layoutSubviews();
    begin({55,114});end({55,114});expect(s->isOn() && changed==5,"reparented switch accepts a new contact after cancelled gesture");
    s->setOn(false);s->setOn(true,true);s->setEnabled(false);UISwitchTestHarness::advance(s,1);
    expect(s->isOn() && changed==5,"disabling during programmatic animation preserves silent target state");s->setEnabled(true);s->setOn(false);
    const auto weak=std::weak_ptr<UISwitch>(s);
    auto removal=UIAction("Remove after changing",[weak]{if(const auto control=weak.lock())control->removeFromSuperview();});
    s->addAction(removal,UIControlEvent::valueChanged);
    begin({55,114});end({55,114});expect(s->isOn() && changed==6 && s->superview().expired(),"value callback may remove control after state invariants are restored");
    s->removeAction(removal,UIControlEvent::valueChanged);
}
void nativeLiftGeometry() {
    // Presentation bounds measured from a real OS hold on native UISwitch.
    // Phase alignment .05375s accounts for the observed input/CA commit offset;
    // it is a comparison alignment, not a delay inserted into the control.
    const std::pair<double,double> samples[]={
        {.0790445,45.96622844},{.0957,50.9623},{.1124,54.8675},
        {.129,57.6411},{.1457,59.3914},{.1624,60.3054},{.179,60.5969},
        {.1957,60.4707},{.2124,60.1021},{.229,59.6276},{.2457,59.1438}
    };
    auto s=new_shared<UISwitch>();UISwitchTestHarness::press(s,true);
    double previous=0,maximum=0;
    for(const auto& [sampleTime,width]:samples) {
        const auto time=sampleTime-.05375;
        UISwitchTestHarness::advance(s,time-previous);previous=time;
        maximum=std::max(maximum,std::abs(double(UISwitchTestHarness::lens(s)->bounds().width())-width));
    }
    expect(maximum<.25,"small shared lens response matches native switch held-size presentation samples");
    UISwitchTestHarness::advance(s,1);
    expect(std::abs(UISwitchTestHarness::lens(s)->bounds().width()-58)<.001f,"switch held size settles to native58pt width");
    UISwitchTestHarness::press(s,false);UISwitchTestHarness::advance(s,1);
    expect(UISwitchTestHarness::lens(s)->bounds().size==NXKit::NXSize(37,24),"release restores native resting bounds");
    std::cout<<"Native UISwitch lift max width residual="<<maximum<<"pt\n";
}
void nativePositionAndLowering() {
    auto s=new_shared<UISwitch>();s->setOn(true,true);
    double previous=0,maximumPosition=0,maximumBounds=0;
    for(const auto& sample:Internal::switchNativeProgrammaticPosition) {
        UISwitchTestHarness::advance(s,sample.time-previous);previous=sample.time;
        maximumPosition=std::max(maximumPosition,std::abs(double(UISwitchTestHarness::position(s))-sample.fraction)*22);
        expect(UISwitchTestHarness::lens(s)->bounds().size==NXKit::NXSize(37,24),"programmatic response keeps native resting knob bounds");
    }
    s->setOn(true);UISwitchTestHarness::press(s,true);UISwitchTestHarness::advance(s,1);
    UISwitchTestHarness::press(s,false);previous=0;
    for(const auto& sample:Internal::switchNativeLoweringBounds) {
        UISwitchTestHarness::advance(s,sample.time-previous);previous=sample.time;
        const auto size=UISwitchTestHarness::lens(s)->bounds().size;
        maximumBounds=std::max({maximumBounds,std::abs(double(size.width)-sample.width),std::abs(double(size.height)-sample.height)});
    }
    expect(maximumPosition<.0001,"native programmatic knob position matches .3s critical response");
    expect(maximumBounds<.0001,"native lowering bounds match the shared small-lens response separately from view-motion flex");
    std::cout<<"Native UISwitch position max residual="<<maximumPosition<<"pt lowering bounds="<<maximumBounds<<"pt\n";
}
void shortTapMinimumLift() {
    unsigned cases=0;
    // Fresh native tap captures reach the complete lift/overshoot (~60.60pt)
    // even when touch-up precedes it. Shared lens spec hang time is .22s.
    for(const auto hz:{60.,120.})for(const bool initiallyOn:{false,true})
        for(const auto duration:{0.,.016,.05,.12,.18,.3,.65}) {
        auto s=new_shared<UISwitch>();s->setOn(initiallyOn);
        auto touch=new_shared<UITouch>(1,NXKit::NXPoint(initiallyOn?42:20,14),Timer());
        int changes=0,ups=0;
        s->addAction(UIAction("Changed",[&]{++changes;}),UIControlEvent::valueChanged);
        s->addAction(UIAction("Up",[&]{++ups;}),UIControlEvent::touchUpInside);
        expect(s->beginTracking(touch,nullptr),"tap begins the native switch contact path");
        double age=0,peak=37;
        auto step=[&](double seconds){UISwitchTestHarness::advance(s,seconds);age+=seconds;
            peak=std::max(peak,double(UISwitchTestHarness::lens(s)->bounds().width()));};
        while(age+1e-10<duration)step(std::min(1./hz,duration-age));
        const auto before=UISwitchTestHarness::lens(s)->bounds().size;
        s->endTracking(touch,nullptr);
        expect(!UISwitchTestHarness::accepted(s) && s->isOn()!=initiallyOn && changes==1 && ups==1,
            "tap commits value/actions immediately while its visual lift may continue");
        expect(UISwitchTestHarness::lens(s)->bounds().size==before,"release does not jump the knob geometry");
        if(duration<.2)expect(UISwitchTestHarness::lens(s)->isLifted(),"short tap retains native shared minimum lift window");
        else if(duration>.3)expect(!UISwitchTestHarness::lens(s)->isLifted(),"settled long contact lowers immediately on release");
        while(age<std::max(.9,duration+.6))step(1./hz);
        expect(peak>60.3 && peak<60.9,"tap completes native full expansion and overshoot before lowering");
        expect(!UISwitchTestHarness::lens(s)->isLifted() && UISwitchTestHarness::lens(s)->bounds().size==NXKit::NXSize(37,24),
            "minimum lift and lowering settle to native resting knob geometry");
        expect(changes==1 && ups==1,"deferred visual lowering does not repeat control actions");++cases;
    }
    for(const bool afterRelease:{false,true})for(const auto elapsed:{0.,.05,.18}) {
        auto s=new_shared<UISwitch>();auto touch=new_shared<UITouch>(1,NXKit::NXPoint(20,14),Timer());
        s->beginTracking(touch,nullptr);UISwitchTestHarness::advance(s,elapsed);
        if(afterRelease){s->endTracking(touch,nullptr);s->setOn(false,false);}
        else s->cancelTracking();
        expect(!UISwitchTestHarness::lens(s)->isLifted() && UISwitchTestHarness::lens(s)->bounds().size==NXKit::NXSize(37,24),
            "cancel or nonanimated programmatic state interrupts pending lift immediately");
        UISwitchTestHarness::advance(s,1);
        expect(!UISwitchTestHarness::lens(s)->isLifted() && UISwitchTestHarness::lens(s)->bounds().size==NXKit::NXSize(37,24),
            "cancelled delayed callback cannot resurrect the knob geometry");++cases;
    }
    for(const auto operation:{0,1,2}) {
        auto parent=new_shared<UIView>();auto s=new_shared<UISwitch>();parent->addSubview(s);
        auto touch=new_shared<UITouch>(1,NXKit::NXPoint(20,14),Timer());int changes=0,cancels=0;
        s->addAction(UIAction("Changed",[&]{++changes;}),UIControlEvent::valueChanged);
        s->addAction(UIAction("Cancel",[&]{++cancels;}),UIControlEvent::touchCancel);
        s->beginTracking(touch,nullptr);s->endTracking(touch,nullptr);UISwitchTestHarness::advance(s,.05);
        expect(UISwitchTestHarness::lens(s)->isLifted(),"released tap still owns its minimum visual lift before lifecycle cancellation");
        if(operation==0)s->cancelTracking();else if(operation==1)s->setEnabled(false);else s->removeFromSuperview();
        expect(!UISwitchTestHarness::lens(s)->isLifted() && UISwitchTestHarness::lens(s)->bounds().size==NXKit::NXSize(37,24),
            "cancellation, disable and removal clear a released tap's deferred visual state");
        UISwitchTestHarness::advance(s,1);
        expect(s->isOn() && changes==1 && cancels==0 && !UISwitchTestHarness::lens(s)->isLifted(),
            "cleaning a completed tap never cancels its completed contact or changes committed state");++cases;
    }
    {
        auto baseline=new_shared<UISwitch>();auto retargeted=new_shared<UISwitch>();
        UISwitchTestHarness::press(baseline,true);UISwitchTestHarness::press(retargeted,true);
        UISwitchTestHarness::advance(baseline,.1);UISwitchTestHarness::advance(retargeted,.1);
        retargeted->setOn(true,true);
        UISwitchTestHarness::advance(baseline,.17);UISwitchTestHarness::advance(retargeted,.17);
        expect(UISwitchTestHarness::lens(baseline)->bounds().size==UISwitchTestHarness::lens(retargeted)->bounds().size,
            "programmatic on animation does not restart the independent lift geometry clock");
        UISwitchTestHarness::press(baseline,false);UISwitchTestHarness::press(retargeted,false);
        UISwitchTestHarness::advance(baseline,.5);UISwitchTestHarness::advance(retargeted,.5);++cases;
    }
    std::cout<<"UISwitch short tap/shared lift cases="<<cases<<'\n';
}
class TraitRoot final:public UIView {
public:
    explicit TraitRoot(std::shared_ptr<UITraitCollection> traits){_traitCollection=std::move(traits);}
    void setTraits(std::shared_ptr<UITraitCollection> traits) {
        const auto previous=_traitCollection;_traitCollection=std::move(traits);traitCollectionDidChange(previous);
    }
};
void attachedTraitTrackColors() {
    const auto previousAmbient=UITraitCollection::current();
    unsigned cases=0;
    for(const auto scale:{1.f,2.f,3.f})for(const bool dark:{false,true})for(const bool enabled:{false,true}) {
        const auto attached=GlassMaterialTestHarness::traits(dark,scale);
        const auto opposite=GlassMaterialTestHarness::traits(!dark,scale);
        UITraitCollection::setCurrent(opposite);
        auto root=new_shared<TraitRoot>(attached);auto s=new_shared<UISwitch>();root->addSubview(s);s->setEnabled(enabled);
        auto check=[&](UIColor on,const char* message) {
            const auto traits=s->traitCollection();
            const auto expected=UIColor::tertiaryLabel.resolvedColorWithTraitCollection(traits).interpolationTo(
                on.resolvedColorWithTraitCollection(traits),UISwitchTestHarness::position(s));
            const auto actual=UISwitchTestHarness::trackColor(s);
            expect(actual && actual->raw()==expected.raw(),message);++cases;
        };
        s->setOn(false);check(UIColor::systemGreen,"off track resolves attached-root traits despite opposite ambient theme");
        s->setOn(true);check(UIColor::systemGreen,"on track resolves attached-root traits despite opposite ambient theme");
        const auto dynamicOn=UIColor([](const auto& traits){return traits->userInterfaceStyle()==UIUserInterfaceStyle::dark?UIColor::systemPink:UIColor::systemBlue;});
        s->setOnTintColor(dynamicOn);check(dynamicOn,"dynamic onTintColor resolves attached-root traits");
        s->setOn(false,true);UISwitchTestHarness::advance(s,.08);
        check(dynamicOn,"moving track resolves both color endpoints against the control's traits");
        // Move only the view hierarchy's theme. The ambient collection remains
        // deliberately different from the newly attached collection as well.
        UITraitCollection::setCurrent(attached);root->setTraits(opposite);
        check(dynamicOn,"live parent trait change updates the interpolated track independently of ambient traits");
        s->setOn(false);check(dynamicOn,"off track updates after a live theme change");
        s->setOn(true);check(dynamicOn,"on tint updates after a live theme change");
        auto destination=new_shared<TraitRoot>(attached);UITraitCollection::setCurrent(opposite);destination->addSubview(s);
        check(dynamicOn,"reparenting updates track colors from the new attached root");
    }
    UITraitCollection::setCurrent(previousAmbient);
    std::cout<<"UISwitch attached-trait color cases="<<cases<<'\n';
}
}
int main(){NXKit::SkiaCtx::_main=std::make_shared<NXKit::Internal::RasterTestContext>();properties();touchTracking();nativeLiftGeometry();nativePositionAndLowering();shortTapMinimumLift();attachedTraitTrackColors();std::cout<<"UISwitch regression failures="<<failures<<'\n';return failures?1:0;}
