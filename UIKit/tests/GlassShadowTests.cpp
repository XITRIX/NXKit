#include <BackdropEffectView.h>
#include <GlassContainerView.h>
#include <CABasicAnimation.h>
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include <algorithm>
#include <cmath>
#include <iostream>
using namespace NXKit;
namespace NXKit {
class GlassMaterialTestHarness {
public:
    static std::shared_ptr<UITraitCollection> traits(bool dark,int scale) {
        auto value=std::make_shared<UITraitCollection>();
        value->_userInterfaceStyle=dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;
        value->_displayScale=scale;return value;
    }
};
}
namespace {
int failures=0;
void expect(bool value,const char* message) { if(!value) { ++failures;std::cerr<<"FAIL: "<<message<<'\n'; } }
class Root final:public UIView {
public:
    Root(bool dark,int scale,UIColor color=UIColor::white):UIView({0,0,480,300}) {
        _traitCollection=GlassMaterialTestHarness::traits(dark,scale);setBackgroundColor(color);
    }
};
UIGlassEffect glass(bool clear=false,int tint=0) {
    UIGlassEffect value(clear?UIGlassEffect::Style::clear:UIGlassEffect::Style::regular);
    value.setFrostRadius(0);value.setRefraction(0);value.setEdgeStrength(0);value.setEdgeAbsorption(0);
    if(tint==1)value.setTintColor(UIColor(0,122,255));
    if(tint==2)value.setTintColor(UIColor(255,69,58));
    return value;
}
std::shared_ptr<BackdropEffectView> item(NXRect frame,bool clear=false,int tint=0) {
    auto view=new_shared<BackdropEffectView>(glass(clear,tint));view->setFrame(frame);
    view->layer()->setCornerRadius(std::min(frame.width(),frame.height())*.5f);return view;
}
SkBitmap render(const std::shared_ptr<UIView>& root,int scale) {
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(480*scale,300*scale));
    surface->getCanvas()->clear(SK_ColorTRANSPARENT);surface->getCanvas()->scale(scale,scale);
    root->layer()->skiaRender(surface->getCanvas());
    SkBitmap result;result.allocN32Pixels(480*scale,300*scale);
    expect(surface->readPixels(result,0,0),"shadow raster is readable");return result;
}
int channel(const SkBitmap& image,NXPoint p,int scale,int shift=16) {
    const auto color=image.getColor(std::floor(p.x*scale),std::floor(p.y*scale));return (color>>shift)&255;
}
bool identical(const SkBitmap& a,const SkBitmap& b) {
    for(int y=0;y<a.height();++y)for(int x=0;x<a.width();++x)if(a.getColor(x,y)!=b.getColor(x,y))return false;
    return true;
}
void testNativeProfiles() {
    // Native framebuffer exterior probes, white background:52/120 circles
    // and240x160 continuous card; two themes. Distances are in logical points.
    for(bool dark:{false,true})for(int scale:{1,2,3})for(int shape:{0,1,2}) {
        auto root=new_shared<Root>(dark,scale);
        const NXRect frames[]={{60,40,52,52},{60,40,120,120},{60,40,240,160}};
        auto view=item(frames[shape]);if(shape==2)view->layer()->setCornerRadius(36);root->addSubview(view);
        const auto image=render(root,scale);const auto frame=frames[shape];
        const int near[]={252,dark?235:241,dark?225:235};
        const int far[]={254,dark?241:246,dark?232:240};
        expect(std::abs(channel(image,{frame.midX(),frame.maxY()+1},scale)-near[shape])<=2,
            "native near-edge elevation profile follows size and theme at every backing scale");
        expect(std::abs(channel(image,{frame.midX(),frame.maxY()+8},scale)-far[shape])<=2,
            "native shadow tail follows point dimensions independently of backing scale");
        if(shape==0)expect(channel(image,{frame.midX(),frame.minY()-4},scale)==255,
            "small glass retains the observed downward shadow without a broad top halo");
    }
}
void testOverridesAndClipping() {
    for(bool dark:{false,true}) {
        auto root=new_shared<Root>(dark,2,UIColor(128,128,128));auto view=item({50,40,120,120});root->addSubview(view);
        const auto normal=render(root,2);view->layer()->setShadowOpacity(0);const auto disabled=render(root,2);
        for(int y=60;y<140;y+=8)for(int x=70;x<150;x+=8)
            expect(normal.getColor(x*2,y*2)==disabled.getColor(x*2,y*2),"glass never shades or refracts its own default shadow");
        expect(channel(disabled,{110,164},2)==128,"shadowOpacity zero removes default shadow immediately");
        view->layer()->setShadowOpacity(.5f);const auto half=render(root,2);
        const auto fullLoss=128-channel(normal,{110,164},2),halfLoss=128-channel(half,{110,164},2);
        expect(std::abs(halfLoss*2-fullLoss)<=2,"shadowOpacity scales native default elevation");
        view->layer()->setShadowOpacity(1);view->layer()->setShadowColor(UIColor::blue);
        view->layer()->setShadowOffset({0,0});view->layer()->setShadowRadius(0);
        expect(channel(render(root,2),{110,164},2)==128,"explicit app shadow suppresses the automatic glass shadow");
        view->layer()->setShadowColor(std::nullopt);view->setEffect(glass(true));
        const auto clear=render(root,2);
        for(float distance:{1,4,8,16})expect(channel(clear,{110,160+distance},2)==128,
            "native clear glass has no automatic shadow in either appearance");
    }
}
void testTintAndBackdropIndependence() {
    for(bool dark:{false,true})for(auto color:{UIColor::white,UIColor(128,128,128),UIColor::red,UIColor::green,UIColor::blue,UIColor(255,255,0)}) {
        SkBitmap neutral;
        for(int tint:{0,1,2}) {
            auto root=new_shared<Root>(dark,2,color);root->addSubview(item({60,40,120,120},false,tint));const auto image=render(root,2);
            if(tint==0)neutral=image;
            for(float distance:{2,8,16,32})expect(image.getColor(120*2,(160+distance)*2)==neutral.getColor(120*2,(160+distance)*2),
                "exterior shadow is black and independent of the material tint and backdrop hue");
        }
    }
}
void testContainerElevation() {
    for(bool dark:{false,true}) {
        auto root=new_shared<Root>(dark,2);auto group=new_shared<GlassContainerView>();group->setFrame(root->bounds());root->addSubview(group);
        auto small=item({40,40,52,52});group->contentView()->addSubview(small);
        const auto single=render(root,2);small->removeFromSuperview();root->addSubview(small);
        expect(identical(single,render(root,2)),"single-item containers preserve the ordinary shadow exactly");
        group->contentView()->addSubview(small);auto large=item({240,40,160,160});group->contentView()->addSubview(large);
        const auto elevated=render(root,2);
        expect(std::abs(channel(elevated,{66,93},2)-(dark?225:235))<=2,
            "native container shares its largest member elevation with disconnected smaller items");
        large->setHidden(true);
        expect(std::abs(channel(render(root,2),{66,93},2)-252)<=1,"hiding the largest member restores small shared elevation");
        large->removeFromSuperview();auto second=item({94,94,52,52});group->contentView()->addSubview(second);
        expect(std::abs(channel(render(root,2),{66,93},2)-252)<=1,"diagonal spacing and aggregate bounds do not raise shared elevation");
        small->setEffect(glass(true));second->setEffect(glass(true));
        expect(channel(render(root,2),{66,93},2)==255,"all-clear containers cast no automatic shadow");
    }
}
void testAffineShadowCoordinates() {
    for(int scale:{1,2,3}) {
        auto root=new_shared<Root>(false,scale);auto group=new_shared<GlassContainerView>();group->setFrame(root->bounds());root->addSubview(group);
        const NXAffineTransform transform(1.25f,.2f,.15f,.8f,0,0);
        auto a=item({50,50,52,52}),b=item({230,50,52,52});a->setTransform(transform);b->setTransform(transform);
        root->addSubview(a);root->addSubview(b);const auto separate=render(root,scale);
        group->contentView()->addSubview(a);group->contentView()->addSubview(b);const auto combined=render(root,scale);
        for(auto centre:{NXPoint(76,76),NXPoint(256,76)})for(float distance:{2,8,16}) {
            const auto point=NXPoint(0,26+distance).applying(transform)+centre;
            expect(std::abs(channel(separate,point,scale)-channel(combined,point,scale))<=1,
                "container and standalone elevation share affine offsets and covariance at every scale");
        }
    }
}
}
int main() {
    testNativeProfiles();testOverridesAndClipping();testTintAndBackdropIndependence();testContainerElevation();testAffineShadowCoordinates();
    if(failures)return 1;std::cout<<"Glass shadow tests passed\n";
}
