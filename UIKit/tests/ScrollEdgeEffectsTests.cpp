// Share only the reproducible scene fixture; this supplies a separate test main.
#define NXKIT_SCROLL_EDGE_REFERENCE_NO_MAIN
#include "../../Demo/ScrollEdgeComparison/PortableReference.cpp"
#include <cmath>
#include <limits>
#include <cstdlib>
#include "ProgressiveBlur.hpp"
#include "include/effects/SkGradient.h"
#include "include/effects/SkImageFilters.h"
#include "include/core/SkShader.h"

using namespace ScrollEdgeReference;
namespace NXKit {
class SizeClassTestHarness {public:static void refresh(const std::shared_ptr<UIWindow>& window){window->updateTraitsFromPlatform();}};
}
namespace {
int failures=0,checks=0;
void expect(bool condition,const std::string& message){++checks;if(!condition){++failures;std::cerr<<"FAIL: "<<message<<'\n';}}
Internal::ScrollPocketSnapshot edge(Scene& scene,int index){return Internal::scrollPocketSnapshot(*scene.screen,scene.screen->view()->superview().lock(),index);}
void states() {
    Scene scene;const auto scroll=scene.screen->scroll;const auto safe=scroll->safeAreaInsets();
    expect(safe==UIEdgeInsets(116,32,83,32),"native fixture safe insets include bars and the screen's custom side insets");
    scroll->setContentOffset({-safe.left,-safe.top},false);
    expect(!edge(scene,0).visible&&!edge(scene,1).visible,"resting top and leading edges are inactive");
    scroll->setContentOffset({-safe.left,1-safe.top},false);
    expect(edge(scene,0).visible&&edge(scene,0).strength>0&&edge(scene,0).strength<1,
        "entering the current safe area begins a gradual edge response");
    scroll->setContentOffset({120,180},false);
    expect(edge(scene,0).visible&&edge(scene,2).visible&&!edge(scene,1).visible&&!edge(scene,3).visible,"side safe insets alone do not invent horizontal edge chrome");
    scene.screen->installSideChrome();settle(scene.window);
    expect(edge(scene,1).visible&&edge(scene,3).visible,"side container interactions activate the corresponding horizontal edges");
    scene.screen->sideContainers.front()->setAlpha(0);
    expect(!edge(scene,1).visible&&edge(scene,3).visible,"transparent custom chrome never activates its edge or suppresses the visible opposite chrome");
    scene.screen->sideContainers.front()->setAlpha(1);
    scroll->topEdgeEffect()->setHidden(true);
    expect(!edge(scene,0).visible&&edge(scene,2).visible,"hiding one edge leaves the opposite effect enabled");
    scroll->topEdgeEffect()->setHidden(false);scroll->topEdgeEffect()->setStyle(UIScrollEdgeEffectStyle::hard);
    expect(edge(scene,0).style==UIScrollEdgeEffectStyle::hard,"explicit hard style reaches the renderer");
    scroll->setContentOffset({-safe.left,-safe.top-24},false);
    expect(!edge(scene,0).visible,"leading overscroll reveals unblurred content instead of activating an edge");
    scroll->setContentSize({0,0});
    expect(!edge(scene,0).visible&&!edge(scene,2).visible,"empty content contributes no scroll edge effect");
}
void liveGeometry(const std::shared_ptr<Context>& context) {
    Scene scene;auto scroll=scene.screen->scroll;
    const auto verify=[&](const std::string& label) {
        scroll->setContentOffset({120,180},false);
        const auto safe=scroll->safeAreaInsets();
        for(const auto style:{UIScrollEdgeEffectStyle::automatic,UIScrollEdgeEffectStyle::soft,UIScrollEdgeEffectStyle::hard}) {
            scroll->topEdgeEffect()->setStyle(style);scroll->bottomEdgeEffect()->setStyle(style);
            const auto top=edge(scene,0),bottom=edge(scene,2);
            expect(top.visible&&top.safeExtent==safe.top&&top.effectExtent==safe.top,
                label+": top pocket follows the scroll view safe area in every style");
            expect(bottom.visible&&bottom.safeExtent==safe.bottom&&bottom.effectExtent==safe.bottom,
                label+": bottom pocket follows the scroll view safe area in every style");
        }
    };
    verify("initial bars");
    scene.screen->setAdditionalSafeAreaInsets({37,32,29,32});settle(scene.window);
    verify("additional attached UI");
    scene.navigation->setNavigationBarHidden(true,false);settle(scene.window);
    verify("hidden navigation bar");
    scene.navigation->setNavigationBarHidden(false,false);settle(scene.window);
    verify("restored navigation bar");
    context->size={874,402};context->safe={0,27,21,19};
    scene.window->setFrame({{},context->size});SizeClassTestHarness::refresh(scene.window);settle(scene.window);
    expect(scroll->bounds().size==context->size,"rotation resizes the mounted scroll viewport");
    verify("landscape device insets and compact bar");
    context->size={1024,768};context->safe={24,0,20,0};
    scene.window->setFrame({{},context->size});SizeClassTestHarness::refresh(scene.window);settle(scene.window);
    verify("tablet-sized viewport");
    scene.screen->viewportInsets={200,80,200,80};scene.screen->view()->setNeedsLayout();settle(scene.window);
    expect(scroll->safeAreaInsets().top==0&&scroll->safeAreaInsets().bottom==0,
        "an inset viewport wholly inside its parent safe area has no vertical occlusion");
    scroll->setContentOffset({120,180},false);
    expect(!edge(scene,0).visible&&!edge(scene,2).visible,
        "zero safe-area extents do not invent a pocket from a bar metric or extension");
    context->size={402,874};context->safe={62,0,34,0};
}
void associatedContainerGeometry() {
    Scene scene;const auto scroll=scene.screen->scroll;
    scroll->setContentOffset({120,180},false);
    const auto size=scroll->bounds().size;
    std::array<std::shared_ptr<UIView>,4> containers;
    for(int index=0;index<4;++index) {
        auto container=new_shared<UIView>();scene.screen->view()->addSubview(container);containers[index]=container;
        auto interaction=std::make_shared<UIScrollEdgeElementContainerInteraction>();
        interaction->setScrollView(scroll);interaction->setEdge(static_cast<UIRectEdge>(1u<<index));container->addInteraction(interaction);
        container->setFrame(bandRect(Internal::ScrollPocketSnapshot{.viewport={{},size},.effectExtent=150,.visible=true},index));
        // A small label must not replace the associated container's geometry.
        auto label=new_shared<UILabel>();label->setText("Edge");label->setFrame({0,0,20,20});container->addSubview(label);
    }
    settle(scene.window);
    for(int index=0;index<4;++index) {
        expect(edge(scene,index).visible&&edge(scene,index).effectExtent==150,
            "each associated edge container supplies its actual bounds");
        containers[index]->setFrame(bandRect(Internal::ScrollPocketSnapshot{.viewport={{},size},.effectExtent=200,.visible=true},index));
        expect(edge(scene,index).effectExtent==200,"moving an attached container updates the existing pocket");
        containers[index]->setFrame(bandRect(Internal::ScrollPocketSnapshot{.viewport={{},size},.effectExtent=1,.visible=true},index));
        const auto safe=scroll->safeAreaInsets();const NXFloat extents[]={safe.top,safe.left,safe.bottom,safe.right};
        expect(edge(scene,index).effectExtent==extents[index],"small chrome cannot shrink an existing safe-area pocket");
        containers[index]->setFrame(bandRect(Internal::ScrollPocketSnapshot{.viewport={{},size},.effectExtent=2000,.visible=true},index));
        expect(edge(scene,index).effectExtent==(index%2?size.width:size.height),"pocket extent is clipped to its viewport");
        containers[index]->removeFromSuperview();
        expect(index%2?!edge(scene,index).visible:edge(scene,index).effectExtent==extents[index],
            "removing attached chrome clears its contribution");
    }
}
void selection() {
    Scene scene(true,false);scene.screen->scroll->setContentOffset({120,180},false);
    expect(!edge(scene,0).visible,"automatic discovery follows the first child chain only");
    scene.screen->setContentScrollView(scene.screen->scroll,NSDirectionalRectEdge::all);
    expect(edge(scene,0).visible&&edge(scene,2).visible,"manual selection supports scroll content behind a preceding sibling");
    scene.screen->scroll->removeFromSuperview();
    expect(!edge(scene,0).visible,"a detached manual scroll selection never contributes a stale effect");
}
void screenOwnership() {
    Scene scene;scene.screen->scroll->setContentOffset({120,180},false);settle(scene.window);
    const auto content=scene.screen->scroll;const auto appMask=new_shared<CALayer>();appMask->setFrame({0,0,402,874});appMask->setBackgroundColor(UIColor::white);content->layer()->setMask(appMask);
    auto sourceHost=scene.screen->view()->superview().lock();const auto sourceOverlay=sourceHost->subviews().back();
    auto target=new_shared<EdgeScreen>();scene.navigation->pushViewController(target,true);scene.window->drawAndLayoutTreeIfNeeded();
    const auto targetHost=target->view()->superview().lock();
    expect(targetHost!=sourceHost&&sourceOverlay->superview().lock()==sourceHost,"push mounts a distinct per-screen overlay and keeps the source overlay on its moving host");
    expect(targetHost->subviews().back()!=sourceOverlay,"each navigation screen renders its own edge layer");
    expect(content->layer()->mask()==appMask,"installing navigation edge fading preserves the app's own CALayer mask");
    settle(scene.window);
    expect(sourceHost->superview().expired(),"finishing push removes the outgoing edge host");
    scene.navigation->popViewController(true);scene.window->drawAndLayoutTreeIfNeeded();
    auto restoredHost=scene.screen->view()->superview().lock();
    expect(restoredHost&&restoredHost!=targetHost,"pop restores the destination content and its edge host together");
    settle(scene.window);expect(edge(scene,0).visible&&content->layer()->mask()==appMask,"pop restores source edge state and preserves the app mask");
    std::weak_ptr<UIView> weakHost=restoredHost;restoredHost.reset();sourceHost.reset();
    scene.navigation->setViewControllers({new_shared<UIViewController>()},false);settle(scene.window);
    expect(weakHost.expired(),"removed edge hosts release their weakly installed content-fade bindings");
    expect(content->layer()->mask()==appMask,"removal never clears an application-owned mask");
}
double difference(const SkBitmap& a,const SkBitmap& b,NXRect points,NXFloat scale=2) {
    double result=0;std::size_t count=0;
    for(int y=std::lround(points.minY()*scale);y<std::lround(points.maxY()*scale);++y)for(int x=std::lround(points.minX()*scale);x<std::lround(points.maxX()*scale);++x){const auto p=a.getColor(x,y),q=b.getColor(x,y);result+=std::abs(int(SkColorGetR(p))-int(SkColorGetR(q)))+std::abs(int(SkColorGetG(p))-int(SkColorGetG(q)))+std::abs(int(SkColorGetB(p))-int(SkColorGetB(q)));++count;}
    return result/(3*count);
}
void runtimeSelectionMatchesInitialSelection() {
    SkBitmap late,initial;
    {Scene scene(true,false);scene.screen->scroll->setContentOffset({120,180},false);render(scene.window);
        scene.screen->setContentScrollView(scene.screen->scroll,NSDirectionalRectEdge::all);late=render(scene.window);}
    {Scene scene(true,true);scene.screen->scroll->setContentOffset({120,180},false);initial=render(scene.window);}
    expect(difference(late,initial,{40,20,320,150})<.5,
        "a content scroll selection changed after mounting installs the same fade and blur as initial selection");
}
void runtimeHierarchyMatchesInitialDiscovery() {
    SkBitmap changed,initial;
    {Scene scene(true,false);scene.screen->scroll->setContentOffset({120,180},false);render(scene.window);
        scene.screen->view()->subviews().front()->removeFromSuperview();changed=render(scene.window);}
    {Scene scene;scene.screen->scroll->setContentOffset({120,180},false);initial=render(scene.window);}
    expect(difference(changed,initial,{40,20,320,150})<.5,
        "changing the first-subview chain after mounting updates fade and blur together");
}
void variableBlur() {
    Scene scene;scene.screen->scroll->setContentOffset({120,180},false);auto baseline=render(scene.window);
    auto probe=new_shared<BackdropEffectView>(UIBlurEffect::effectWithVariableBlurRadius(24,imageMask("zero")));probe->setFrame({24,220,354,256});scene.window->addSubview(probe);
    const auto zero=render(scene.window);expect(difference(baseline,zero,{40,235,320,225})<.25,"zero-alpha mask leaves the backdrop unchanged");
    probe->setEffect(UIBlurEffect::effectWithVariableBlurRadius(24,imageMask("one")));const auto uniform=render(scene.window);
    expect(difference(baseline,uniform,{40,235,320,225})>5,"opaque mask applies a visible full-strength blur");
    probe->setEffect(UIBlurEffect::effectWithVariableBlurRadius(24,imageMask("ramp")));const auto ramp=render(scene.window);
    const auto top=difference(baseline,ramp,{40,225,320,24}),bottom=difference(baseline,ramp,{40,445,320,24});
    expect(top<bottom,"a gradient mask progressively increases the local blur radius");
    expect(difference(ramp,uniform,{40,225,320,80})>1,"progressive blur differs from applying uniform blur through a replacement fade");
    bool rejected=false;try{UIBlurEffect::effectWithVariableBlurRadius(-1,imageMask("one"));}catch(const std::invalid_argument&){rejected=true;}
    expect(rejected,"variable blur rejects invalid radii before mutating its installed effect");
}
void sparseContentHasNoInnerBoundaryMatte() {
    Scene scene(false,false,false,true);scene.screen->scroll->setContentOffset({120,180},false);
    const auto edges={scene.screen->scroll->topEdgeEffect(),scene.screen->scroll->leftEdgeEffect(),scene.screen->scroll->bottomEdgeEffect(),scene.screen->scroll->rightEdgeEffect()};
    for(const auto& value:edges)value->setHidden(true);const auto hidden=render(scene.window);
    for(const auto& value:edges)value->setHidden(false);const auto soft=render(scene.window);
    // This location is a transparent tile on both sides of the soft band's
    // interior boundary, away from text, controls, and checker transitions.
    const auto end=edge(scene,0).effectExtent;const auto y0=int(std::floor(end))-2,y1=int(std::ceil(end))+2;
    const auto before=soft.getColor(376,y0*2),after=soft.getColor(376,y1*2),reference=hidden.getColor(376,y0*2);
    const auto sampleY=int(std::lround(end*.35f))*2;
    const auto gap=soft.getColor(192,sampleY),gapReference=hidden.getColor(192,sampleY);
    expect(std::abs(int(SkColorGetR(gap))-int(SkColorGetR(gapReference)))<4
        &&std::abs(int(SkColorGetG(gap))-int(SkColorGetG(gapReference)))<4
        &&std::abs(int(SkColorGetB(gap))-int(SkColorGetB(gapReference)))<4,
        "transparent sparse cells keep the screen's white background inside the active soft band");
    const auto colored=soft.getColor(312,sampleY),coloredReference=hidden.getColor(312,sampleY);
    expect(SkColorGetR(colored)>SkColorGetR(coloredReference)+5
        &&SkColorGetG(colored)>=SkColorGetG(coloredReference)
        &&SkColorGetB(colored)>=SkColorGetB(coloredReference)-2,
        "sparse colored content fades toward the screen background instead of an opaque black matte");
    expect(std::abs(int(SkColorGetR(before))-int(SkColorGetR(reference)))<8
        &&std::abs(int(SkColorGetG(before))-int(SkColorGetG(reference)))<8
        &&std::abs(int(SkColorGetB(before))-int(SkColorGetB(reference)))<8,
        "transparent content near the inner edge does not acquire an opaque compositing matte");
    expect(std::abs(int(SkColorGetR(before))-int(SkColorGetR(after)))<8
        &&std::abs(int(SkColorGetG(before))-int(SkColorGetG(after)))<8
        &&std::abs(int(SkColorGetB(before))-int(SkColorGetB(after)))<8,
        "the soft band's inner boundary has no hard pixel jump across a transparent content gap");
}
void contentCallbacksRunOncePerFrame() {
    class CountingLayer final:public CALayer {
        std::shared_ptr<std::array<int,2>> counter;
    public:
        explicit CountingLayer(std::shared_ptr<std::array<int,2>> count):counter(std::move(count)){}
        explicit CountingLayer(CountingLayer* source):CALayer(source),counter(source->counter){}
        std::shared_ptr<CALayer> copy()override{return new_shared<CountingLayer>(this);}
        void draw(SkCanvas* canvas)override {
            ++(*counter)[0];if(!canvas->getSurface())++(*counter)[1];
            SkPaint paint;paint.setColor(UIColor::systemBlue.raw());
            canvas->drawRect(SkRect::MakeWH(bounds().width(),bounds().height()),paint);
        }
    };
    Scene scene;auto scroll=scene.screen->scroll;
    auto original=scroll->subviews().front();original->removeFromSuperview();
    auto counter=std::make_shared<std::array<int,2>>();
    scroll->addSubview(new_shared<UIView>(NXRect{0,0,864,1536},new_shared<CountingLayer>(counter)));
    scroll->setContentOffset({120,180},false);
    render(scene.window);
    expect((*counter)[0]==1&&(*counter)[1]==0,"ScrollPocket draws content once on the live canvas without a recording/analysis replay");
    render(scene.window);
    expect((*counter)[0]==2&&(*counter)[1]==0,"subsequent ScrollPocket frames keep drawing directly without recording content");
}
void constantPocketMask(const std::shared_ptr<Context>& context) {
    for(bool dark:{false,true}) {
        context->dark=dark;
        std::array<SkColor,2> samples;
        for(int sparse=0;sparse<2;++sparse) {
            Scene scene;auto scroll=scene.screen->scroll;
            scroll->subviews().front()->removeFromSuperview();
            auto red=new_shared<UIView>(sparse?NXRect{120,180,160,100}:NXRect{0,0,864,1536});
            red->setBackgroundColor(UIColor::red);scroll->addSubview(red);
            scroll->setContentOffset({120,180},false);
            // Isolate the content mask from the intentionally stronger blur.
            scene.screen->view()->superview().lock()->subviews().back()->setHidden(true);
            samples[sparse]=render(scene.window).getColor(80,40);
        }
        const int background=dark?0:255;
        const int expectedGreen=std::lround(background*.6f);
        expect(samples[0]==samples[1],"dense and sparse content receive the same ScrollPocket fade");
        expect(std::abs(int(SkColorGetG(samples[0]))-expectedGreen)<=2,
            "the outer ScrollPocket mask keeps 0.4 content opacity in either appearance");
    }
    context->dark=false;
}
void mountedMaskTracksSafeArea() {
    Scene scene;const auto scroll=scene.screen->scroll;
    scroll->subviews().front()->removeFromSuperview();
    auto red=new_shared<UIView>(NXRect{{},scroll->contentSize()});red->setBackgroundColor(UIColor::red);scroll->addSubview(red);
    scroll->setContentOffset({120,180},false);
    scene.screen->view()->superview().lock()->subviews().back()->setHidden(true);
    const auto originalExtent=scroll->safeAreaInsets().top;
    const int x=80,y=std::lround((originalExtent+1)*2);
    const auto initial=render(scene.window).getColor(x,y);
    expect(SkColorGetG(initial)==0,"content beyond the current safe-area pocket remains unfaded");
    scene.screen->setAdditionalSafeAreaInsets({originalExtent,32,0,32});
    const auto expanded=render(scene.window).getColor(x,y);
    expect(SkColorGetG(expanded)>SkColorGetG(initial)+50,
        "the mounted content mask expands when attached UI enlarges the safe area");
    scene.screen->setAdditionalSafeAreaInsets({0,32,0,32});
    expect(render(scene.window).getColor(x,y)==initial,
        "the mounted content mask restores sharp content when additional UI is removed");
}
void strongerPocketBlur() {
    class Stripes final:public CALayer {
    public:
        Stripes()=default;
        explicit Stripes(Stripes* source):CALayer(source){}
        std::shared_ptr<CALayer> copy()override{return new_shared<Stripes>(this);}
        void draw(SkCanvas* canvas)override {
            SkPaint paint;
            for(int x=0;x<864;x+=2){paint.setColor((x/2)%2?SK_ColorBLACK:SK_ColorWHITE);canvas->drawRect(SkRect::MakeXYWH(x,0,2,1536),paint);}
        }
    };
    Scene scene;auto scroll=scene.screen->scroll;scroll->subviews().front()->removeFromSuperview();
    scroll->addSubview(new_shared<UIView>(NXRect{0,0,864,1536},new_shared<Stripes>()));
    scroll->setContentOffset({120,180},false);
    const auto overlay=scene.screen->view()->superview().lock()->subviews().back();
    overlay->setHidden(true);const auto fadeOnly=render(scene.window);
    overlay->setHidden(false);const auto blurred=render(scene.window);
    const auto contrast=[](const SkBitmap& image,int y){
        int low=255,high=0;
        for(int x=80;x<240;++x){const auto value=int(SkColorGetR(image.getColor(x,y)));low=std::min(low,value);high=std::max(high,value);}
        return high-low;
    };
    expect(contrast(fadeOnly,40)>90&&contrast(blurred,40)<10,
        "outer ScrollPocket blur strongly smooths two-point detail rather than merely fading it");
    expect(difference(fadeOnly,blurred,{40,edge(scene,0).effectExtent+1,80,30})<.5,
        "ScrollPocket blur leaves content beyond its inner boundary sharp");
}
void pocketKernelNarrowsWithGradient() {
    Scene scene;auto scroll=scene.screen->scroll;
    scroll->subviews().front()->removeFromSuperview();
    scene.screen->view()->setBackgroundColor(UIColor::black);scroll->setBackgroundColor(UIColor::black);
    auto impulse=new_shared<UIView>(NXRect{200,0,1,1536});impulse->setBackgroundColor(UIColor::white);scroll->addSubview(impulse);
    scene.navigation->navigationBar()->setAlpha(0);
    scroll->setContentOffset({120,180},false);
    const auto image=render(scene.window);
    const auto variance=[&](float fraction) {
        const auto y=int(edge(scene,0).effectExtent*fraction*2);
        double mass=0,mean=0,second=0;
        for(int x=110;x<212;++x){const auto value=SkColorGetR(image.getColor(x,y));mass+=value;mean+=value*x;second+=value*x*x;}
        return second/mass-std::pow(mean/mass,2);
    };
    const auto outer=variance(.25f),middle=variance(.85f);
    // Half mask alpha must halve sigma, hence quarter the kernel variance.
    // Fading a full-radius blur over sharp content instead halves variance
    // and leaves a sharp central impulse: that implementation fails here.
    expect(std::isfinite(outer)&&outer>1&&middle/outer>.12&&middle/outer<.38,
        "mounted ScrollPocket narrows its blur kernel with gradient alpha instead of fading a uniform blur");
}
void pocketFadeHasSoftTail(const std::shared_ptr<Context>& context) {
    context->dark=true;context->scale=3;
    Scene scene;const auto scroll=scene.screen->scroll;
    scroll->subviews().front()->removeFromSuperview();
    scene.screen->view()->setBackgroundColor(UIColor::black);scroll->setBackgroundColor(UIColor::black);
    auto white=new_shared<UIView>(NXRect{{},scroll->contentSize()});white->setBackgroundColor(UIColor::white);scroll->addSubview(white);
    scroll->setContentOffset({120,180},false);
    scene.navigation->navigationBar()->setAlpha(0);scene.tabs->tabBar()->setAlpha(0);
    // Measure attenuation alone, without convolution or colored reference cells.
    scene.screen->view()->superview().lock()->subviews().back()->setHidden(true);
    const auto image=render(scene.window,3);
    for(int index : {0,2}) {
        const auto snapshot=edge(scene,index);const auto extent=snapshot.effectExtent;
        const auto value=[&](float t) {
            const auto y=index==0?t*extent:scroll->bounds().height()-t*extent;
            return int(SkColorGetR(image.getColor(200*3,std::clamp(int(y*3),0,image.height()-1))));
        };
        int largestStep=0;
        for(int distance=1;distance<int(extent);++distance)
            largestStep=std::max(largestStep,value(distance/extent)-value((distance-1)/extent));
        expect((largestStep-1)*extent/255<2,
            "soft attenuation avoids a concentrated high-slope band at edge "+std::to_string(index));
        expect(value(.875f)<254&&value(.99f)>=254,
            "soft attenuation eases into full clarity at the live boundary instead of finishing abruptly early");
    }
    context->dark=false;context->scale=2;
}
void titleBandBlur(const std::shared_ptr<Context>& context) {
    const auto destination=std::getenv("NXKIT_SCROLL_POCKET_PICTURES");
    for(bool dark:{false,true})for(int scale:{1,2,3}) {
        context->dark=dark;context->scale=scale;
        Scene scene;auto scroll=scene.screen->scroll;
        scene.screen->setTitle("Shared");
        scroll->subviews().front()->removeFromSuperview();
        scene.screen->view()->setBackgroundColor(UIColor::black);scroll->setBackgroundColor(UIColor::black);
        const auto bar=scene.navigation->navigationBar();
        const auto center=bar->convertToView({bar->bounds().midX(),bar->bounds().midY()},scroll);
        const auto titleY=center.y-scroll->bounds().minY();
        auto impulse=new_shared<UIView>(NXRect{200,0,1,1536});impulse->setBackgroundColor(UIColor::white);scroll->addSubview(impulse);
        bar->setAlpha(0);scroll->setContentOffset({120,180},false);
        const auto image=render(scene.window,scale);
        const auto variance=[&](float y) {
            double mass=0,mean=0,second=0;
            for(int x=50*scale;x<110*scale;++x){const auto value=SkColorGetR(image.getColor(x,std::lround(y*scale)));mass+=value;mean+=value*x;second+=value*x*x;}
            return second/mass-std::pow(mean/mass,2);
        };
        const auto outer=variance(edge(scene,0).effectExtent*.25f),title=variance(titleY);
        const auto label=std::to_string(scale)+"x "+(dark?"dark":"light");
        // A title must have substantial blur behind it before the outer edge
        // reaches its maximum. Check the mounted bar's actual center, rather
        // than selecting a convenient point in the implementation's gradient.
        expect(title/outer>.40&&title/outer<.85,
            "title-band kernel suppresses readable detail while retaining progression ("+label+", variance ratio="+std::to_string(title/outer)+")");
        expect(std::abs(outer/(scale*scale)-16)<2,
            "earlier blur onset retains the four-point maximum kernel ("+label+")");
        if(destination) {
            impulse->removeFromSuperview();bar->setAlpha(1);
            scene.screen->view()->setBackgroundColor(UIColor::systemBackground);scroll->setBackgroundColor(UIColor::systemBackground);
            auto card=new_shared<UIView>(NXRect{132,180,378,194});card->setBackgroundColor(UIColor::secondarySystemGroupedBackground);card->layer()->setCornerRadius(22);scroll->addSubview(card);
            for(int row=0;row<3;++row) {
                auto text=new_shared<UILabel>();text->setAutolayoutEnabled(false);text->setText(row==0?"Push next: Accent action":row==1?"Pop to collection":"Push any arrangement");
                text->setFontSize(17);text->setTextColor(UIColor::systemTeal);text->setFrame({148,180+titleY-12+row*48,330,24});scroll->addSubview(text);
            }
            const auto picture=render(scene.window,scale);
            const std::filesystem::path directory=destination;std::filesystem::create_directories(directory);
            SkFILEWStream file((directory/("title-"+std::to_string(scale)+"-"+(dark?"dark":"light")+".png")).c_str());
            expect(SkPngEncoder::Encode(&file,picture.pixmap(),{}),"mounted text-under-title comparison encodes successfully");
        }
    }
    context->dark=false;context->scale=2;
}
void progressiveKernelProfile() {
    for(int scale:{1,2,3})for(bool vertical:{false,true})for(bool reverse:{false,true}) {
        const int size=128*scale;
        const SkPoint points[]={{0,0},vertical?SkPoint{0,float(size)}:SkPoint{float(size),0}};
        const SkColor4f colors[]={{1,1,1,reverse?0.f:1.f},{1,1,1,reverse?1.f:0.f}};
        auto mask=SkShaders::LinearGradient(points,SkGradient(SkGradient::Colors(colors,{},SkTileMode::kClamp),{}));
        auto surface=SkSurfaces::Raster(SkImageInfo::Make(size,size,kRGBA_F32_SkColorType,kPremul_SkAlphaType));
        auto canvas=surface->getCanvas();canvas->clear(SK_ColorBLACK);SkPaint effect;effect.setImageFilter(Internal::progressiveEdgeBlurFilter(4,mask,{128,128},scale,vertical));
        const auto bounds=SkRect::MakeWH(size,size);canvas->saveLayer(&bounds,&effect);canvas->clear(SK_ColorBLACK);
        SkPaint ink;ink.setColor(SK_ColorWHITE);
        // A three-pixel impulse keeps low-valued Gaussian tails measurable
        // through Skia's intermediate surfaces. Its discrete variance is 2/3.
        canvas->drawRect(vertical?SkRect::MakeXYWH(size/2-1,0,3,size):SkRect::MakeXYWH(0,size/2-1,size,3),ink);canvas->restore();
        SkPixmap pixels;surface->peekPixels(&pixels);
        for(float fraction:{.25f,.5f,.75f}) {
            const int row=int(size*fraction);
            const float intensity=reverse?(row+.5f)/size:1-(row+.5f)/size;
            const auto sigma=4*scale*intensity;
            double mass=0,mean=0,second=0;
            for(int column=0;column<size;++column){const auto value=vertical?pixels.getColor4f(column,row).fR:pixels.getColor4f(row,column).fR;mass+=value;mean+=value*column;second+=value*column*column;}
            const auto variance=second/mass-std::pow(mean/mass,2);
            const auto expected=sigma*sigma+2./3.;
            expect(std::isfinite(variance)&&std::abs(variance-expected)<.2+.09*sigma*sigma,
                "progressive kernel variance follows mask alpha squared: scale="+std::to_string(scale)+" vertical="+std::to_string(vertical)+" reverse="+std::to_string(reverse)+" fraction="+std::to_string(fraction)+" actual="+std::to_string(variance)+" expected="+std::to_string(expected));
        }
        if(scale>2) {
            const auto normalizedPeak=[&](int row) {
                double mass=0,peak=0;
                for(int column=0;column<size;++column){const auto value=vertical?pixels.getColor4f(column,row).fR:pixels.getColor4f(row,column).fR;mass+=value;peak=std::max(peak,double(value));}
                const auto alpha=reverse?(row+.5f)/size:1-(row+.5f)/size;
                return peak/mass*(4*scale*alpha);
            };
            expect(std::abs(normalizedPeak(size/2-1)-normalizedPeak(size/2))<.025,
                "resolution transition keeps a continuous kernel profile across the radius threshold");
        }
        effect.setImageFilter(Internal::progressiveEdgeBlurFilter(4,SkShaders::Color(SK_ColorTRANSPARENT),{128,128},scale,vertical));
        canvas->clear(SK_ColorBLACK);canvas->saveLayer(&bounds,&effect);canvas->clear(SK_ColorBLACK);
        canvas->drawRect(vertical?SkRect::MakeXYWH(size/2,0,1,size):SkRect::MakeXYWH(0,size/2,size,1),ink);canvas->restore();
        surface->peekPixels(&pixels);
        const auto center=pixels.getColor4f(size/2,size/2).fR;
        const auto adjacent=vertical?pixels.getColor4f(size/2+1,size/2).fR:pixels.getColor4f(size/2,size/2+1).fR;
        expect(center==1&&adjacent==0,"zero kernel radius preserves the exact sharp source at every backing scale");
        effect.setImageFilter(Internal::progressiveEdgeBlurFilter(4,SkShaders::Color(SK_ColorWHITE),{128,128},scale,vertical));
        canvas->clear(SK_ColorBLACK);canvas->saveLayer(&bounds,&effect);canvas->clear(SK_ColorWHITE);canvas->restore();surface->peekPixels(&pixels);
        expect(pixels.getColor4f(0,0).fR>.99f&&pixels.getColor4f(size-1,size-1).fR>.99f,
            "progressive blur preserves constant color at the viewport boundary");
    }
}
void kernelReferenceImages() {
    const auto destination=std::getenv("NXKIT_PROGRESSIVE_BLUR_PICTURES");if(!destination)return;
    const std::filesystem::path directory=destination;std::filesystem::create_directories(directory);
    const SkPoint points[]={{0,0},{0,256}};const SkColor4f colors[]={{1,1,1,1},{1,1,1,0}};
    const auto mask=SkShaders::LinearGradient(points,SkGradient(SkGradient::Colors(colors,{},SkTileMode::kClamp),{}));
    for(bool progressive:{false,true}) {
        auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(384,256));auto canvas=surface->getCanvas();canvas->clear(SK_ColorBLACK);
        SkPaint ink;ink.setColor(SK_ColorWHITE);
        for(int x=8;x<112;x+=8)canvas->drawRect(SkRect::MakeXYWH(x,0,2,256),ink);
        SkFont font(SkiaCtx::main()->getDefaultTypeface(),26);
        for(int y=36;y<256;y+=48){const char* text="Progressive blur";canvas->drawSimpleText(text,16,SkTextEncoding::kUTF8,128,y,font,ink);}
        canvas->save();if(!progressive)canvas->clipShader(mask);
        SkPaint paint;paint.setBlendMode(SkBlendMode::kSrc);
        paint.setImageFilter(progressive?Internal::progressiveEdgeBlurFilter(4,mask,{192,128},2,true):SkImageFilters::Blur(8,8,SkTileMode::kClamp,nullptr));
        const auto bounds=SkRect::MakeWH(384,256);
        canvas->saveLayer(SkCanvas::SaveLayerRec(&bounds,&paint,SkCanvas::kInitWithPrevious_SaveLayerFlag));canvas->restore();canvas->restore();
        SkBitmap image;image.allocN32Pixels(384,256);surface->readPixels(image,0,0);
        SkFILEWStream file((directory/(progressive?"variable-kernel.png":"masked-uniform.png")).c_str());
        expect(SkPngEncoder::Encode(&file,image.pixmap(),{}),"progressive kernel comparison image encodes successfully");
    }
}
void adaptiveGlassKeepsLiveBackdropObservation() {
    Scene scene;auto scroll=scene.screen->scroll;scroll->setContentOffset({120,180},false);
    auto black=new_shared<UIView>(NXRect{160,400,200,120});black->setBackgroundColor(UIColor::black);scroll->addSubview(black);
    UIGlassEffect effect;effect.setAdaptsAppearanceToBackdrop(true);
    auto glass=new_shared<BackdropEffectView>(effect);glass->setFrame({180,420,120,64});scroll->addSubview(glass);
    auto ink=new_shared<UIView>(NXRect{20,20,8,8});ink->setBackgroundColor(UIColor::label);glass->addSubview(ink);
    expect(edge(scene,0).visible&&edge(scene,2).visible,"adaptive glass fixture keeps both vertical scroll edge effects active");
    for(int frame=0;frame<3;++frame)render(scene.window);
    expect(ink->traitCollection()->userInterfaceStyle()==UIUserInterfaceStyle::dark,
        "opt-in glass inside faded scroll content observes its actual black sibling backdrop");
    expect(scene.screen->view()->traitCollection()->userInterfaceStyle()==UIUserInterfaceStyle::light,
        "adaptive glass leaves its light-mode screen's own appearance unchanged");
}
void traits(const std::shared_ptr<Context>& context) {
    context->dark=true;context->scale=3;
    {Scene scene;scene.screen->scroll->setContentOffset({120,180},false);settle(scene.window);
        const auto overlay=scene.screen->view()->superview().lock()->subviews().back();
        expect(scene.screen->view()->traitCollection()->userInterfaceStyle()==UIUserInterfaceStyle::dark,"edge hosts inherit the screen's dark appearance");
        expect(overlay->layer()->contentsScale()==3,"edge layers use the screen's Retina backing scale");
        auto image=render(scene.window,3);expect(image.width()==1206&&image.height()==2622,"the same edge geometry renders at 3x without changing logical bounds");
        context->dark=false;context->scale=2;SizeClassTestHarness::refresh(scene.window);settle(scene.window);
        expect(overlay->layer()->contentsScale()==2,"a live display-scale change updates the existing edge layer");
        expect(scene.screen->view()->traitCollection()->userInterfaceStyle()==UIUserInterfaceStyle::light,
            "a live theme change reaches the existing screen and its fade provider");
    }
    context->dark=false;context->scale=2;
}
}
int main(){
    auto context=std::make_shared<Context>();SkiaCtx::_main=context;UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
    states();liveGeometry(context);associatedContainerGeometry();selection();screenOwnership();runtimeSelectionMatchesInitialSelection();runtimeHierarchyMatchesInitialDiscovery();variableBlur();sparseContentHasNoInnerBoundaryMatte();contentCallbacksRunOncePerFrame();constantPocketMask(context);mountedMaskTracksSafeArea();strongerPocketBlur();pocketKernelNarrowsWithGradient();pocketFadeHasSoftTail(context);titleBandBlur(context);progressiveKernelProfile();kernelReferenceImages();adaptiveGlassKeepsLiveBackdropObservation();traits(context);
    UIApplication::shared.reset();SkiaCtx::_main.reset();
    if(!failures)std::cout<<"Scroll edge effects: "<<checks<<" checks passed\n";return failures?1:0;
}
