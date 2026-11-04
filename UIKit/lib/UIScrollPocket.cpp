#include "UIScrollPocket.hpp"
#include "ViewControllerContentScrollAccess.hpp"
#include "LayerContentMask.hpp"
#include "ProgressiveBlur.hpp"
#include "include/core/SkShader.h"
#include "include/effects/SkGradient.h"
#include "include/effects/SkImageFilters.h"
#include <algorithm>
#include <cmath>
#include <UIScrollEdgeElementContainerInteraction.h>
using namespace NXKit;
namespace {
constexpr NSDirectionalRectEdge edges[]={NSDirectionalRectEdge::top,NSDirectionalRectEdge::leading,NSDirectionalRectEdge::bottom,NSDirectionalRectEdge::trailing};
constexpr NXFloat softRadius=4;
std::shared_ptr<UIScrollEdgeEffect> effectFor(const std::shared_ptr<UIScrollView>& scroll,int edge) {
    switch(edge){case 0:return scroll->topEdgeEffect();case 1:return scroll->leftEdgeEffect();case 2:return scroll->bottomEdgeEffect();default:return scroll->rightEdgeEffect();}
}
bool finite(NXFloat value){return std::isfinite(value);}
struct Chrome {std::shared_ptr<UIView> container;std::shared_ptr<UIScrollView> scroll;};
Chrome findChrome(const std::shared_ptr<UIView>& root,int edge) {
    if(!root||root->isHidden()||root->alpha()<=0)return {};
    for(const auto& candidate:root->interactions())if(const auto interaction=std::dynamic_pointer_cast<UIScrollEdgeElementContainerInteraction>(candidate)) {
        if((static_cast<unsigned>(interaction->edge())&(1u<<edge))&&interaction->scrollView())return {root,interaction->scrollView()};
    }
    for(const auto& child:root->subviews())if(!child->isHidden()){const auto found=findChrome(child,edge);if(found.container)return found;}
    return {};
}
NXRect edgeRect(NXSize size,int edge,NXFloat extent) {
    switch(edge){case 0:return {0,0,size.width,extent};case 1:return {0,0,extent,size.height};case 2:return {0,size.height-extent,size.width,extent};default:return {size.width-extent,0,extent,size.height};}
}
float blurRecoveryAt(float t) {
    // Preserve the established title-band kernel widths, but join their
    // anchors with monotone cubic Hermite segments instead of straight lines.
    constexpr float stops[]={0,.50f,.65f,.80f,.90f,1};
    constexpr float values[]={0,0,.08f,.35f,.70f,1};
    constexpr auto tangents=[=] {
        std::array<float,6> result{};
        for(int i=1;i<5;++i) {
            const auto left=(values[i]-values[i-1])/(stops[i]-stops[i-1]);
            const auto right=(values[i+1]-values[i])/(stops[i+1]-stops[i]);
            result[i]=left>0&&right>0?2*left*right/(left+right):0;
        }
        return result;
    }();
    int index=0;while(index<4&&t>stops[index+1])++index;
    const auto width=stops[index+1]-stops[index];
    const auto u=std::clamp((t-stops[index])/width,0.f,1.f),u2=u*u,u3=u2*u;
    return (2*u3-3*u2+1)*values[index]+(u3-2*u2+u)*width*tangents[index]
        +(-2*u3+3*u2)*values[index+1]+(u3-u2)*width*tangents[index+1];
}
sk_sp<SkShader> profileGradient(NXSize size,int edge,NXFloat extent,float outer,float inner,NXFloat scale,bool fadesContent) {
    SkPoint points[2];
    switch(edge){case 0:points[0]={0,0};points[1]={0,extent};break;
        case 1:points[0]={0,0};points[1]={extent,0};break;
        case 2:points[0]={0,size.height};points[1]={0,size.height-extent};break;
        default:points[0]={size.width,0};points[1]={size.width-extent,0};}
    for(auto& point:points){point.fX*=scale;point.fY*=scale;}
    // Bake easing into the cached gradient. This needs no additional effect
    // pass or per-frame content analysis. Both ends flatten into their adjacent
    // constant region; the attenuation tail now reaches the live inner bound.
    // Kernel and attenuation remain separate so text beneath titles stays blurred.
    std::array<float,33> stops;
    std::array<SkColor4f,33> colors;
    for(size_t i=0;i<stops.size();++i) {
        const auto t=float(i)/float(stops.size()-1);stops[i]=t;
        const auto u=std::clamp((t-.25f)/.75f,0.f,1.f);
        const auto recovery=fadesContent?u*u*u*(u*(u*6-15)+10):blurRecoveryAt(t);
        colors[i]={1,1,1,outer+(inner-outer)*recovery};
    }
    return SkShaders::LinearGradient(points,SkGradient(SkGradient::Colors(SkSpan(colors),SkSpan(stops),SkTileMode::kClamp),{}));
}
bool isSoft(const Internal::ScrollPocketSnapshot& state){return state.style!=UIScrollEdgeEffectStyle::hard;}
}
Internal::ScrollPocketSnapshot Internal::scrollPocketSnapshot(UIViewController& controller,const std::shared_ptr<UIView>& host,int edge) {
    ScrollPocketSnapshot result;if(!host||edge<0||edge>3||!controller.viewIsLoaded())return result;
    const auto chrome=findChrome(controller.view(),edge);
    auto scroll=chrome.scroll?chrome.scroll:ViewControllerContentScrollAccess::resolvedContentScrollView(controller,edges[edge]);
    if(edge%2&&!chrome.container)return result;
    if(!scroll||!scroll->window()||scroll->window()!=host->window())return result;
    auto ancestor=std::static_pointer_cast<UIView>(scroll);
    while(ancestor&&ancestor!=controller.view())ancestor=ancestor->superview().lock();
    if(!ancestor)return result;
    for(auto node=std::static_pointer_cast<UIView>(scroll);node;node=node->superview().lock())if(node->isHidden()||node->alpha()<=0)return result;
    const auto effect=effectFor(scroll,edge);if(effect->isHidden())return result;
    const auto size=scroll->bounds().size;const auto content=scroll->contentSize();
    if(!finite(size.width)||!finite(size.height)||!finite(content.width)||!finite(content.height)
       ||size.width<=0||size.height<=0||content.width<=0||content.height<=0)return result;
    const auto safe=scroll->safeAreaInsets();const NXFloat extents[]={safe.top,safe.left,safe.bottom,safe.right};
    auto extent=extents[edge];
    const auto origin=scroll->convertToView(scroll->bounds().origin,host);
    if(!finite(origin.x)||!finite(origin.y))return result;
    if(chrome.container) {
        const NXRect containerRect{chrome.container->convertToView(chrome.container->bounds().origin,host),chrome.container->bounds().size};
        const NXFloat reach[]={containerRect.maxY()-origin.y,containerRect.maxX()-origin.x,origin.y+size.height-containerRect.minY(),origin.x+size.width-containerRect.minX()};
        if(!finite(reach[edge]))return result;
        extent=std::max(extent,reach[edge]);
    }
    if(!finite(extent)||extent<=0)return result;
    extent=std::min(extent,edge%2?size.width:size.height);
    const auto offset=scroll->layer()->presentationOrSelf()->bounds().origin;
    // The current safe area includes device occlusion, containing bars and
    // additional controller insets. Associated edge containers can extend it;
    // neither style nor a reference device supplies an extra layout distance.
    const NXFloat distances[]={offset.y+extent,offset.x+extent,
        content.height+extent-size.height-offset.y,content.width+extent-size.width-offset.x};
    const auto distance=distances[edge];if(!finite(distance)||distance<=0)return result;
    result.scroll=scroll;result.viewport={origin,size};result.safeExtent=extent;
    result.strength=std::clamp(distance/extent,0.f,1.f);result.style=effect->style();
    result.effectExtent=extent;
    result.visible=true;return result;
}
namespace {
// One private pocket per physical edge, matching UIKit's _UIScrollPocket
// terminology. Public configuration remains UIScrollEdgeEffect. A pocket never
// samples content to decide its opacity, blur radius or gradient.
class UIScrollPocket final {
    struct GradientCache {
        NXSize size;
        NXFloat extent=0,strength=-1,scale=0;
        sk_sp<SkShader> shader;
    } fade,blur;
    const int edge;
    NXFloat filterScale=0;
    bool filterSoft=false;
    sk_sp<SkShader> filterMask;
    sk_sp<SkImageFilter> filter;
    sk_sp<SkShader> gradient(GradientCache& cache,NXSize size,NXFloat extent,
                             NXFloat strength,NXFloat scale,bool fadesContent) {
        if(!cache.shader||cache.size!=size||cache.extent!=extent
           ||cache.strength!=strength||cache.scale!=scale) {
            cache.size=size;cache.extent=extent;cache.strength=strength;cache.scale=scale;
            cache.shader=profileGradient(size,edge,extent,
                fadesContent?1-.6f*strength:strength,fadesContent?1:0,scale,fadesContent);
        }
        return cache.shader;
    }
public:
    explicit UIScrollPocket(int value):edge(value){}
    sk_sp<SkShader> contentMask(NXSize size,const Internal::ScrollPocketSnapshot& snapshot) {
        return gradient(fade,size,snapshot.effectExtent,snapshot.strength,1,true);
    }
    sk_sp<SkShader> blurMask(const Internal::ScrollPocketSnapshot& snapshot,NXFloat scale) {
        return gradient(blur,snapshot.viewport.size,snapshot.effectExtent,snapshot.strength,scale,false);
    }
    sk_sp<SkImageFilter> blurFilter(const Internal::ScrollPocketSnapshot& snapshot,NXFloat scale,bool soft) {
        const auto mask=soft?blurMask(snapshot,scale):nullptr;
        if(!filter||filterScale!=scale||filterSoft!=soft||filterMask!=mask) {
            filterScale=scale;filterSoft=soft;
            filterMask=mask;
            filter=soft?Internal::progressiveEdgeBlurFilter(softRadius,mask,snapshot.viewport.size,scale,edge%2==0)
                       :SkImageFilters::Blur(16*scale,16*scale,SkTileMode::kClamp,nullptr);
        }
        return filter;
    }
};
struct ScrollPocketState {
    std::weak_ptr<UIViewController> controller;
    std::weak_ptr<UIView> host;
    std::array<UIScrollPocket,4> pockets{UIScrollPocket(0),UIScrollPocket(1),UIScrollPocket(2),UIScrollPocket(3)};
};
class ScrollPocketMask final:public Internal::LayerContentMask {
    std::shared_ptr<ScrollPocketState> state;
    mutable std::array<sk_sp<SkShader>,4> masks;
    mutable sk_sp<SkShader> combined;
public:
    explicit ScrollPocketMask(std::shared_ptr<ScrollPocketState> s):state(std::move(s)){}
    bool affectsSubtree() const override {return true;}
    bool active(const CALayer& layer) const override {
        const auto controller=state->controller.lock();const auto host=state->host.lock();const auto view=std::dynamic_pointer_cast<UIScrollView>(layer.delegate.lock());
        if(!controller||!host||!view)return false;
        for(int edge=0;edge<4;++edge){const auto snapshot=Internal::scrollPocketSnapshot(*controller,host,edge);if(snapshot.visible&&snapshot.scroll==view&&isSoft(snapshot))return true;}return false;
    }
    sk_sp<SkShader> contentShader(const CALayer& layer) const override {
        std::array<sk_sp<SkShader>,4> next;
        const auto controller=state->controller.lock();const auto host=state->host.lock();const auto view=std::dynamic_pointer_cast<UIScrollView>(layer.delegate.lock());if(!controller||!host||!view)return SkShaders::Color(SK_ColorWHITE);
        for(int edge=0;edge<4;++edge) {
            const auto snapshot=Internal::scrollPocketSnapshot(*controller,host,edge);if(!snapshot.visible||snapshot.scroll!=view||!isSoft(snapshot))continue;
            next[edge]=state->pockets[edge].contentMask(layer.bounds().size,snapshot);
        }
        if(!combined||!std::equal(next.begin(),next.end(),masks.begin(),[](const auto& a,const auto& b){return a.get()==b.get();})) {
            masks=std::move(next);combined.reset();
            for(const auto& mask:masks)if(mask)combined=combined?SkShaders::Blend(SkBlendMode::kDstIn,std::move(combined),mask):mask;
            if(!combined)combined=SkShaders::Color(SK_ColorWHITE);
        }
        return combined;
    }
    void draw(SkCanvas* canvas,const CALayer& layer) const override {
        SkPaint paint;paint.setShader(contentShader(layer));
        canvas->drawRect(SkRect::MakeWH(layer.bounds().width(),layer.bounds().height()),paint);
    }
};
class ScrollPocketBackdropLayer final:public CALayer {
    std::shared_ptr<ScrollPocketState> state;
public:
    explicit ScrollPocketBackdropLayer(std::shared_ptr<ScrollPocketState> s):state(std::move(s)){}
    explicit ScrollPocketBackdropLayer(ScrollPocketBackdropLayer* source):CALayer(source),state(source->state){}
    std::shared_ptr<CALayer> copy() override{return new_shared<ScrollPocketBackdropLayer>(this);}
    void draw(SkCanvas* canvas) override {
        const auto controller=state->controller.lock();const auto host=state->host.lock();if(!controller||!host)return;
        const auto scale=std::max(1.f,contentsScale());
        for(int edge=0;edge<4;++edge) {
            const auto snapshot=Internal::scrollPocketSnapshot(*controller,host,edge);if(!snapshot.visible)continue;
            const auto size=snapshot.viewport.size;
            const auto soft=isSoft(snapshot);const auto extent=snapshot.effectExtent;
            const auto rect=edgeRect(size,edge,extent);
            canvas->save();canvas->translate(snapshot.viewport.minX(),snapshot.viewport.minY());canvas->clipRect(SkRect::MakeXYWH(rect.minX(),rect.minY(),rect.width(),rect.height()));
            canvas->scale(1/scale,1/scale);
            SkPaint paint;
            paint.setImageFilter(state->pockets[edge].blurFilter(snapshot,scale,soft));
            // The gradient controls kernel sigma inside the filter. Replace
            // the backdrop directly; fading a uniformly blurred copy would
            // leave a sharp image superimposed on the blurred one.
            if(soft)paint.setBlendMode(SkBlendMode::kSrc);
            const auto viewport=SkRect::MakeWH(size.width*scale,size.height*scale);
            canvas->saveLayer(SkCanvas::SaveLayerRec(&viewport,&paint,SkCanvas::kInitWithPrevious_SaveLayerFlag));canvas->restore();
            canvas->scale(scale,scale);
            if(!soft) {
                SkPaint tint;const auto background=UIColor::systemBackground;auto color=background.raw();
                tint.setColor(SkColorSetA(color,std::lround(190*snapshot.strength)));canvas->drawRect(SkRect::MakeXYWH(rect.minX(),rect.minY(),rect.width(),rect.height()),tint);
                SkPaint line;line.setColor(UIColor::separator.raw());const auto pixel=1/scale;
                auto divider=edgeRect(size,edge,extent);if(edge==0)divider={0,extent-pixel,size.width,pixel};else if(edge==1)divider={extent-pixel,0,pixel,size.height};else if(edge==2)divider={0,size.height-extent,size.width,pixel};else divider={size.width-extent,0,pixel,size.height};
                canvas->drawRect(SkRect::MakeXYWH(divider.minX(),divider.minY(),divider.width(),divider.height()),line);
            }
            canvas->restore();
        }
    }
};
class ScrollPocketHost final:public UIView {
    std::shared_ptr<ScrollPocketState> state=std::make_shared<ScrollPocketState>();
    std::shared_ptr<UIView> overlay;
    std::weak_ptr<CALayer> boundRootLayer;
    std::shared_ptr<ScrollPocketMask> contentFade;
public:
    explicit ScrollPocketHost(const std::shared_ptr<UIViewController>& controller) {
        state->controller=controller;state->host=shared_from_this();
        overlay=new_shared<UIView>(NXRect{},new_shared<ScrollPocketBackdropLayer>(state));overlay->setUserInteractionEnabled(false);
        // Install after content is mounted; the overlay never enters discovery.
    }
    void update() {
        const auto controller=state->controller.lock();if(!controller)return;
        if(controller->viewIsLoaded()) {
            const auto rootLayer=controller->view()->layer();
            if(boundRootLayer.lock()!=rootLayer) {
                // Resolve the active scroll at composition time throughout the
                // subtree, including selections and hierarchy changes made
                // after mounting. Drawing never changes these bindings.
                contentFade=std::make_shared<ScrollPocketMask>(state);
                Internal::LayerContentMaskAccess::add(rootLayer,contentFade);
                boundRootLayer=rootLayer;
            }
        }
        if(overlay->superview().expired())addSubview(overlay);
        overlay->setFrame(bounds());
        overlay->layer()->setContentsScale(traitCollection()?traitCollection()->displayScale():1);
    }
    void layoutSubviews() override {UIView::layoutSubviews();update();}
};
}
std::shared_ptr<UIView> Internal::makeScrollPocketHost(const std::shared_ptr<UIViewController>& controller){
    // Containers install the decoration on each actual content screen, avoiding
    // a second effect when navigation stacks are nested inside tabs.
    if(std::dynamic_pointer_cast<UINavigationControllerInterface>(controller)||std::dynamic_pointer_cast<UITabBarControllerInterface>(controller))return new_shared<UIView>();
    return new_shared<ScrollPocketHost>(controller);
}
void Internal::updateScrollPocketHost(const std::shared_ptr<UIView>& host){if(const auto edge=std::dynamic_pointer_cast<ScrollPocketHost>(host))edge->update();}
