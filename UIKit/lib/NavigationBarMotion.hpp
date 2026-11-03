#pragma once
#include <UIView.h>
#include <CABasicAnimation.h>
#include <UIViewAnimationGroup.h>
#include "include/effects/SkImageFilters.h"
#include <algorithm>
#include <cmath>
#include <initializer_list>

namespace NXKit::Internal {
inline auto navigationAnimation(const char* property) {
    auto animation=new_shared<CABasicAnimation>(property);
    animation->animationGroup=new_shared<UIViewAnimationGroup>(UIViewAnimationOptions(allowUserInteraction|preferredFramesPerSecond120),std::nullopt);
    return animation;
}
inline double navigationSpring(double t,double damping=.72,double frequency=16) {
    if(t<=0)return 0;const auto d=std::sqrt(1-damping*damping);
    return 1-std::exp(-damping*frequency*t)*(std::cos(frequency*d*t)+damping/d*std::sin(frequency*d*t));
}
inline double navigationSurfacePulse(double t) {
    return navigationSpring(t,.59,17.6)-navigationSpring(t-.092,.59,17.6);
}
// One shared clock for controller content and navigation titles. The near-
// critically damped native slide has a fast departure and a long settling tail.
inline constexpr double navigationContentDuration=.5;
inline double navigationContentProgress(double t) {
    const auto u=std::max(0.,t-.0325)*22.7;
    return 1-std::exp(-u)*(1+u);
}
struct NavigationKeyframe {double time,value;};
// Cubic Hermite interpolation keeps measured optical phases continuous at
// 120 Hz. These are timing curves, independent of the bar's layout dimensions.
inline double navigationCurve(double t,std::initializer_list<NavigationKeyframe> keys) {
    const auto first=keys.begin(),end=keys.end();if(t<=first->time)return first->value;
    for(auto b=first+1;b!=end;++b)if(t<=b->time) {
        const auto a=b-1;const auto span=b->time-a->time,u=(t-a->time)/span;
        const auto ma=a==first?0.:(b->value-(a-1)->value)/(b->time-(a-1)->time);
        const auto mb=b+1==end?0.:((b+1)->value-a->value)/((b+1)->time-a->time);
        return (2*u*u*u-3*u*u+1)*a->value+(u*u*u-2*u*u+u)*span*ma+(-2*u*u*u+3*u*u)*b->value+(u*u*u-u*u)*span*mb;
    }
    return (end-1)->value;
}
inline double navigationLeadingInflation(double t) {
    return navigationCurve(t,{{0,0},{.07,0},{.125,.032},{.175,.13},{.225,.20},{.275,.165},{.325,.08},{.4,-.017},{.475,-.022},{.55,-.009},{.625,.001},{.7,.003},{.85,0},{.97,0}});
}
inline double navigationLeadingDisplacement(double t) {
    return navigationCurve(t,{{0,0},{.125,0},{.175,2},{.225,5.5},{.275,6.7},{.325,6.1},{.4,3.5},{.475,1.6},{.55,.45},{.65,.03},{.8,0},{.97,0}});
}
inline double navigationLeadingReveal(double t) {
    return navigationCurve(t,{{0,0},{.25,0},{.3,.2},{.35,.46},{.4,.69},{.45,.85},{.5,.94},{.55,1.02},{.6,1.04},{.7,1.02},{.85,1},{.97,1}});
}
inline double navigationLeadingTravel(double t) {
    return navigationCurve(t,{{0,0},{.25,0},{.3,.29},{.35,.55},{.4,.76},{.45,.90},{.5,.965},{.55,1.03},{.6,1.04},{.7,1.02},{.85,1},{.97,1}});
}
inline double navigationLeadingAbsorption(double t) {
    return navigationCurve(t,{{0,0},{.07,0},{.1,.025},{.15,.18},{.2,.47},{.25,.78},{.3,.93},{.35,1},{.4,1}});
}
// Navigation-only foreground filtering. The glass and hit-testing ancestors
// stay sharp; only the outgoing/incoming ink is isolated and filtered.
class NavigationInkLayer final : public CALayer {
public:
    NavigationInkLayer()=default;
    explicit NavigationInkLayer(NavigationInkLayer* other):CALayer(other),_blur(other->_blur){}
    std::shared_ptr<CALayer> copy() override{return new_shared<NavigationInkLayer>(this);}
    NXFloat blur() const{return _blur;}
protected:
    void update(std::shared_ptr<CALayer> presentation,std::shared_ptr<CABasicAnimation> animation,float progress) override {
        if(animation->keyPath=="navigationInkBlur"){
            if(!animation->fromValue||!animation->toValue)return;
            const auto from=std::any_cast<NXFloat>(&*animation->fromValue),to=std::any_cast<NXFloat>(&*animation->toValue);
            if(from&&to)std::static_pointer_cast<NavigationInkLayer>(presentation)->_blur=std::max(0.f,*from+(*to-*from)*progress);
        }else CALayer::update(std::move(presentation),std::move(animation),progress);
    }
    void renderSublayers(SkCanvas* canvas) override {
        if(_blur<.01f){CALayer::renderSublayers(canvas);return;}
        SkPaint paint;paint.setImageFilter(SkImageFilters::Blur(_blur,_blur,nullptr));
        const auto area=SkRect::MakeLTRB(-_blur*4,-_blur*4,bounds().width()+_blur*4,bounds().height()+_blur*4);
        canvas->saveLayer(&area,&paint);CALayer::renderSublayers(canvas);canvas->restore();
    }
private:NXFloat _blur=0;
};
class NavigationInkView final : public UIView {
public:NavigationInkView():UIView({},new_shared<NavigationInkLayer>()){}
};
inline void navigationInkBlur(const std::shared_ptr<UIView>& view,NXFloat from,NXFloat to,double duration,double delay=0) {
    auto animation=navigationAnimation("navigationInkBlur");animation->fromValue=from;animation->toValue=to;animation->duration=duration;animation->delay=delay;
    animation->timingFunction=new_shared<CAMediaTimingFunction>(kCAMediaTimingFunctionEaseInEaseOut);view->layer()->add(animation,"navigationInkBlur");
}
}
