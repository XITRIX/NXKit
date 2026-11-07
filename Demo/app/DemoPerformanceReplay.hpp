#pragma once
#include "Screens/GlassDemoViewController/GlassScreenViewController.hpp"
#include "../../UIKitCore/lib/RenderPerformance.hpp"
#include "../../UIKit/lib/CABackdropEffectLayer.hpp"
#include "../../UIKit/lib/UITabBarViewInternal.hpp"
#include <CADisplayLink.h>
#include <SkiaCtx.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <string_view>

namespace NXKit::DemoPerformance {
inline std::shared_ptr<CADisplayLink>& replay() {
    // Construct after CADisplayLink's global registry so early app shutdown
    // invalidates this link before the registry is destroyed.
    static std::shared_ptr<CADisplayLink> value;
    return value;
}
inline void isolate(const std::shared_ptr<UIView>& view, bool noGlass, bool noPockets) {
    if(noGlass)if(const auto glass=std::dynamic_pointer_cast<BackdropEffectView>(view);glass&&glass->glassEffect()) {
        // Diagnostic control only: keep the actual content, hierarchy, layout
        // and bars, replace material shading with a backdrop identity sample.
        static const BackdropEffect identity("uniform shader content; half4 main(float2 p){return content.eval(p);}");
        // Leave the component-owned UIGlassEffect description intact: the
        // navigation bar uses it when updating tint and group ownership.
        const auto layer=std::static_pointer_cast<CABackdropEffectLayer>(glass->layer());
        if(layer->effect().shaderSource()!=identity.shaderSource())layer->setEffect(identity);
    }
    if(noPockets)if(const auto scroll=std::dynamic_pointer_cast<UIScrollView>(view)) {
        scroll->topEdgeEffect()->setStyle(UIScrollEdgeEffectStyle::none);
        scroll->bottomEdgeEffect()->setStyle(UIScrollEdgeEffectStyle::none);
        scroll->leftEdgeEffect()->setStyle(UIScrollEdgeEffectStyle::none);
        scroll->rightEdgeEffect()->setStyle(UIScrollEdgeEffectStyle::none);
    }
    for(const auto& child:view->subviews())isolate(child,noGlass,noPockets);
}
// Opt-in live tab lens replay. Uses normal dirty rendering and presentation;
// no synchronous readback or forced continuous frames. Does not switch content.
inline void startLens(const std::shared_ptr<GlassScreenViewController>& demo) {
    const auto start=std::chrono::steady_clock::now();
    const auto requested=std::getenv("NXKIT_IOS_DEMO_LENS_SECONDS");
    const auto parsed=requested?std::atof(requested):60.;
    const auto duration=std::isfinite(parsed)?std::clamp(parsed,10.,600.):60.;
    replay()=new_shared<CADisplayLink>([weak=std::weak_ptr<GlassScreenViewController>(demo),start,duration,previous=-1]()mutable {
        const auto screen=weak.lock();if(!screen){replay().reset();return;}
        const auto seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
        if(seconds<2){Internal::RenderPerformance::phase="lens-warmup";return;}
        auto tabs=screen->tabBar();
        const auto elapsed=seconds-2;
        if(elapsed>=duration) {
            Internal::RenderPerformance::phase="lens-idle";
            Internal::TabBarViewAccess::lift(*tabs,false,true);replay().reset();return;
        }
        const auto step=int(elapsed/10),phase=int(elapsed)%10;
        const auto key=step*10+phase;
        if(key!=previous) {
            if(phase==0)Internal::TabBarViewAccess::lift(*tabs,false,false);
            if(phase==1)Internal::TabBarViewAccess::lift(*tabs,true,true);
            if(phase==7)Internal::TabBarViewAccess::lift(*tabs,false,true);
            previous=key;
        }
        Internal::RenderPerformance::phase=phase==0?"lens-rest":phase<3?"lens-lift":phase<7?"lens-drag":"lens-lower";
        if(phase>=3&&phase<7) {
            CALayer::requestFramerate(SkiaCtx::main()->screenFrameRate());
            const auto count=tabs->items().size();
            Internal::TabBarViewAccess::position(*tabs,NXFloat(count-1)*NXFloat(.5-.5*std::cos((elapsed-3)*2*3.141592653589793/2)));
        }
    });
}
// Real application run loop and live geometry, rather than an offscreen GPU
// completion benchmark. Finite: twelve seconds scrolling, eight push/pop pairs,
// another scroll pass, then idle. No forced continuous rendering or FPS cap.
inline void start(const std::shared_ptr<GlassScreenViewController>& demo) {
    if(const auto flag=std::getenv("NXKIT_IOS_DEMO_LENS_REPLAY");flag&&flag[0]=='1'){startLens(demo);return;}
    const auto start=std::chrono::steady_clock::now();
    const auto enabled=[](const char* name){const auto v=std::getenv(name);return v&&std::string_view(v)=="1";};
    const auto noGlass=enabled("NXKIT_IOS_DEMO_NO_GLASS"), noPockets=enabled("NXKIT_IOS_DEMO_NO_POCKETS");
    const auto durationValue=std::getenv("NXKIT_IOS_DEMO_SCROLL_SECONDS");
    const auto scrollSeconds=durationValue?std::clamp(std::atof(durationValue),12.0,600.0):12.0;
    const auto idleResume=enabled("NXKIT_IOS_DEMO_IDLE_RESUME");
    replay()=new_shared<CADisplayLink>([weak=std::weak_ptr<GlassScreenViewController>(demo),start,noGlass,noPockets,scrollSeconds,idleResume,step=-1]()mutable {
        const auto screen=weak.lock();if(!screen){replay().reset();return;}
        if(noGlass||noPockets)isolate(screen->view(),noGlass,noPockets);
        const auto seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
        if(seconds<2){Internal::RenderPerformance::phase="warmup";return;}
        // Slow startup or shader capture can skip a one-second navigation
        // step. Always restore the content root before measuring scrolling;
        // otherwise the replay can report an idle navigation screen as a scroll.
        if(seconds>=30&&step>=0) {
            screen->selectedNavigationController()->popToRootViewController(false);
            step=-2;
            Internal::RenderPerformance::phase="navigation-reset";
            return;
        }
        const auto resumeStart=30+scrollSeconds+15;
        const bool resuming=idleResume&&seconds>=resumeStart&&seconds<resumeStart+12;
        if(seconds<14 || (seconds>=30 && seconds<30+scrollSeconds) || resuming) {
            Internal::RenderPerformance::phase=resuming?"scroll-resume":(seconds<14?"scroll-first":"scroll-second");
            CALayer::requestFramerate(SkiaCtx::main()->screenFrameRate());
            if(const auto list=screen->selectedList()) {
                const auto scroll=list->scrollView();const auto insets=scroll->adjustedContentInset();
                const auto distance=std::max(0.f,scroll->contentSize().height-scroll->bounds().height()+insets.top+insets.bottom);
                const auto local=resuming?seconds-resumeStart:(seconds<14?seconds-2:seconds-30);
                list->setScrollPosition(distance*NXFloat(.5-.5*std::cos(local*2*3.141592653589793/6)));
            }
        } else if(seconds<30) {
            Internal::RenderPerformance::phase="navigation";
            const auto next=int(seconds-14);
            if(next!=step) {
                step=next;const auto nav=screen->selectedNavigationController();
                if(next%2==0)nav->pushViewController(new_shared<GlassNavigationDemoViewController>(size_t(next/2)),true);
                else nav->popViewController(true);
            }
        } else {
            Internal::RenderPerformance::phase="idle";
            if(!idleResume||seconds>=resumeStart+12)replay().reset();
        }
    });
}
}
