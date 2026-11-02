#include <BackdropEffectView.h>
#include <GlassContainerView.h>
#include <CABasicAnimation.h>
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
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
void expect(bool condition,const char* message) {
    if(!condition) {++failures;std::cerr<<"FAIL: "<<message<<'\n';}
}
class Root final:public UIView {
public:
    Root(bool dark,int scale,UIColor background):UIView({0,0,360,240}) {
        _traitCollection=GlassMaterialTestHarness::traits(dark,scale);
        setBackgroundColor(background);
    }
};
std::shared_ptr<BackdropEffectView> item(NXRect frame,UIGlassEffect effect=UIGlassEffect()) {
    auto view=new_shared<BackdropEffectView>(effect);view->setFrame(frame);
    view->layer()->setCornerRadius(std::min(frame.width(),frame.height())*.5f);
    view->layer()->setShadowOpacity(0);return view;
}
SkBitmap render(const std::shared_ptr<UIView>& root,int scale) {
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(360*scale,240*scale));
    surface->getCanvas()->clear(SK_ColorTRANSPARENT);surface->getCanvas()->scale(scale,scale);
    root->layer()->skiaRender(surface->getCanvas());
    SkBitmap pixels;pixels.allocN32Pixels(360*scale,240*scale);
    expect(surface->readPixels(pixels,0,0),"glass contour framebuffer is readable");return pixels;
}
int red(const SkBitmap& image,float x,float y,int scale) {
    return SkColorGetR(image.getColor(std::floor(x*scale),std::floor(y*scale)));
}
void testNativeExteriorProfiles() {
    // Independent native2x neutral regular controls: a half-point exterior
    // contour at the side, attenuated at the top/bottom reflection poles.
    // The previous inside-only rim implementation fails all side probes.
    for(bool dark:{false,true})for(int background:{128,255})for(bool grouped:{false,true}) {
        auto root=new_shared<Root>(dark,2,UIColor(background,background,background));
        std::shared_ptr<UIView> host=root;
        if(grouped) {
            auto group=new_shared<GlassContainerView>();group->setFrame(root->bounds());root->addSubview(group);
            host=group->contentView();host->addSubview(item({220,40,52,52}));
        }
        host->addSubview(item({60,40,52,52}));const auto pixels=render(root,2);
        const int side=background==128?66:(dark?116:190);
        const int top=background==128?(dark?128:109):(dark?254:235);
        expect(std::abs(red(pixels,59.75f,66.25f,2)-side)<=5,
            "light/dark native exterior side contour survives on gray and white, standalone and grouped");
        expect(std::abs(red(pixels,86.25f,39.75f,2)-top)<=3,
            "exterior contour weakens at the native directional reflection poles");
        expect(std::abs(red(pixels,59.25f,66.25f,2)-background)<=1,
            "the contour ends within one point instead of becoming a broad dark border");
        if(!dark&&background==255)expect(red(pixels,86.25f,40.25f,2)>=253,
            "pale regular glass retains its tight inner white reflection");
    }
}
void testPointWidthAndOverrides() {
    for(bool dark:{false,true})for(int scale:{1,2,3}) {
        auto root=new_shared<Root>(dark,scale,UIColor::white);
        auto view=item({60,40,132,56});root->addSubview(view);
        const auto pixels=render(root,scale);
        double loss=0;for(int x=56*scale;x<60*scale;++x)loss+=255-SkColorGetR(pixels.getColor(x,68*scale));
        loss/=scale;
        expect(std::abs(loss-(dark?70:32))<8,
            "integrated exterior contour keeps its point width across1x2x3x rather than expanding by a device pixel");
        UIGlassEffect disabled;disabled.setEdgeStrength(0);view->setEffect(disabled);
        expect(red(render(root,scale),60-0.5f/scale,68,scale)==255,
            "disabling edge strength removes the contour independently of shadowOpacity");
        view->setEffect(UIGlassEffect(UIGlassEffect::Style::clear));
        expect(red(render(root,scale),60-0.5f/scale,68,scale)==255,
            "clear glass keeps its existing exterior without a regular-material contour");
    }
}
void testDarkSizeAndColorControls() {
    // Native exterior side samples at the calibrated sizes.
    const UIColor backgrounds[]={UIColor::white,UIColor(128,128,128),UIColor::green,UIColor(255,255,0)};
    const int native[3][4]={{116,66,169,129},{90,64,144,102},{57,53,108,68}};
    for(int shape=0;shape<3;++shape)for(int color=0;color<4;++color) {
        const float size=shape==0?52:(shape==1?80:120);
        auto root=new_shared<Root>(true,2,backgrounds[color]);auto view=item({60,40,size,size});
        view->layer()->setShadowOpacity(1);root->addSubview(view);const auto pixels=render(root,2);
        const auto c=pixels.getColor(119,std::floor((40+size*.5f)*2));
        const int measured=color==2?SkColorGetG(c):SkColorGetR(c);
        expect(std::abs(measured-native[shape][color])<=6,
            "native dark contour density follows size and backdrop brightness across52/80/120pt geometry");
    }
}
void testMergedSilhouetteAndPresentation() {
    for(bool dark:{false,true}) {
        auto root=new_shared<Root>(dark,2,UIColor(128,128,128));
        auto group=new_shared<GlassContainerView>();group->setFrame(root->bounds());root->addSubview(group);
        auto a=item({60,40,80,56}),b=item({120,40,80,56});group->contentView()->addSubview(a);group->contentView()->addSubview(b);
        const auto joined=render(root,2);
        expect(std::abs(red(joined,119.75f,68,2)-red(joined,130.25f,68,2))<=1,
            "merged members have no internal contour at their former edges");
        b->setHidden(true);const auto hidden=render(root,2);
        expect(red(hidden,200.25f,68,2)==128,"hiding a member removes its exterior contour");
        b->setHidden(false);b->removeFromSuperview();root->addSubview(b);
        auto animation=new_shared<CABasicAnimation>("position");animation->duration=1;
        animation->creationTime=Timer(0);animation->fromValue=NXPoint(260,68);animation->toValue=b->layer()->position();
        b->layer()->add(animation,"contour-position");b->layer()->animateAt(Timer(1));const auto presented=render(root,2);
        const auto position=b->layer()->presentationOrSelf()->position();
        const float edgeX=position.x-b->bounds().width()*.5f;
        expect(red(presented,edgeX-.25f,position.y+.25f,2)<100,
            "reparented glass draws its contour at the animated presentation position");
        expect(red(presented,199.75f,68.25f,2)==128,
            "the old model position does not retain an exterior contour during animation");
        b->layer()->animateAt(Timer(2000));
        b->layer()->removeAllAnimations();
    }
}
}
int main() {
    testNativeExteriorProfiles();testDarkSizeAndColorControls();testPointWidthAndOverrides();testMergedSilhouetteAndPresentation();
    if(failures)return 1;std::cout<<"Glass contour tests passed\n";
}
