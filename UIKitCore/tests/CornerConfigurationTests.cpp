#include <UIKitCore.h>
#include <SkiaCtx.h>
#include <UIWindow.h>
#include "include/core/SkSurface.h"
#include "include/core/SkBitmap.h"
#include <cmath>
#include <iostream>
#include <limits>
using namespace NXKit;
namespace {
int failures=0;
void expect(bool value,const char* message) { if(!value){++failures;std::cerr<<"FAIL: "<<message<<'\n';} }
bool near(float a,float b){return std::abs(a-b)<.01f;}
const UIRectCorner corners[]={UIRectCorner::topLeft,UIRectCorner::topRight,UIRectCorner::bottomLeft,UIRectCorner::bottomRight};
void radii(const std::shared_ptr<UIView>& view,std::array<float,4> expected,const char* message) {
    for(size_t i=0;i<4;++i)expect(near(view->effectiveRadius(corners[i]),expected[i]),message);
}
class Context:public SkiaCtx {
public:
    std::array<NXFloat,4> radii{62,54,46,38};
    NXSize getSize()override{return {300,400};}
    std::array<NXFloat,4> windowCornerRadii()override{return radii;}
    sk_sp<SkSurface> getBackbufferSurface()override{return nullptr;}
    void swapBuffers()override{} bool platformRunLoop(std::function<bool()>)override{return false;}
protected:
    skgpu::graphite::Context* graphiteContext()override{return nullptr;}
    skgpu::graphite::Recorder* graphiteRecorder()override{return nullptr;}
};
void resolution() {
    auto empty=new_shared<UIView>();
    empty->setCornerConfiguration(UICornerConfiguration::capsule());radii(empty,{0,0,0,0},"empty capsules have square geometry");
    empty->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::containerConcentric(12)));radii(empty,{0,0,0,0},"empty views clamp concentric minimum to zero");
    auto parent=new_shared<UIView>(NXRect(0,0,300,400));
    auto child=new_shared<UIView>(NXRect(10,20,260,340));
    parent->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::fixed(60)));
    child->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::containerConcentric()));
    parent->addSubview(child);radii(child,{40,30,20,20},"unequal-axis insets agree with native UIKit");
    expect(near(child->effectiveRadius(UIRectCorner::topLeft|UIRectCorner::bottomRight),40),"multiple corners return maximum");
    expect(child->effectiveRadius(UIRectCorner::none)==0,"empty mask returns zero");
    auto wrapper=new_shared<UIView>(parent->bounds());parent->addSubview(wrapper);wrapper->addSubview(child);
    radii(child,{40,30,20,20},"square wrappers preserve corner container");
    wrapper->layer()->setCornerRadius(40);
    radii(child,{40,30,20,20},"legacy layer radius does not establish a configuration container");
    radii(wrapper,{0,0,0,0},"effective radius queries corner configuration, not legacy layer radius");
    wrapper->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::fixed(0)));
    radii(child,{0,0,0,0},"explicit square configurations establish a container");
    wrapper->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::fixed(40)));
    radii(child,{20,10,0,0},"configured wrapper becomes nearest container");
    parent->addSubview(child);
    child->setTransform(NXAffineTransform::translationBy(20,30));radii(child,{10,10,30,50},"model translation agrees with native effective corner geometry");
    child->setTransform(NXAffineTransform::identity);
    child->setCornerConfiguration(UICornerConfiguration::uniformCorners(UICornerRadius::containerConcentric()));
    radii(child,{40,40,40,40},"uniform corners choose largest resolved radius");
    child->setCornerConfiguration(UICornerConfiguration::uniformTopRadius(UICornerRadius::containerConcentric(),UICornerRadius::fixed(3),UICornerRadius::fixed(7)));
    radii(child,{40,40,3,7},"uniform top preserves independent bottom corners");
    child->setCornerConfiguration(UICornerConfiguration::configurationWithUniformLeftRadius(UICornerRadius::containerConcentric(),UICornerRadius::fixed(9)));
    radii(child,{40,9,40,9},"left/right uniform edge groups resolve separately");
    child->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::fixed(900)));
    radii(child,{130,130,130,130},"fixed radii clamp to shorter dimension");
    child->setCornerConfiguration(UICornerConfiguration::capsule(25));radii(child,{25,25,25,25},"capsule respects maximum");
    child->setCornerConfiguration(UICornerConfiguration::capsule());child->setBounds({0,0,80,120});radii(child,{40,40,40,40},"capsule updates on bounds changes");
    child->setCornerConfiguration(UICornerConfiguration::corners(std::nullopt,UICornerRadius::fixed(12),std::nullopt,std::nullopt));radii(child,{0,12,0,0},"omitted corners are square");
    child->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::containerConcentric(8)));
    child->removeFromSuperview();radii(child,{8,8,8,8},"detached views fall back to minimum");
    auto other=new_shared<UIView>(NXRect(0,0,300,400));other->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::fixed(30)));
    child->setFrame({10,10,280,380});other->addSubview(child);radii(child,{20,20,20,20},"reparenting recomputes container");
    other->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::fixed(40)));radii(child,{30,30,30,30},"ancestor changes update descendants immediately");
    other->setBounds({5,5,300,400});radii(child,{35,25,25,25},"bounds origin participates in corner insets");
    expect(child->applyXMLAttribute("cornerConfiguration","capsule"),"XML accepts capsule configuration");
    expect(child->applyXMLAttribute("cornerConfiguration","containerConcentric"),"XML accepts concentric configuration");
    expect(child->applyXMLAttribute("cornerConfiguration","12"),"XML accepts fixed configuration");
    expect(!child->applyXMLAttribute("cornerConfiguration","invalid")&&!child->applyXMLAttribute("cornerConfiguration","-5"),"XML rejects unsupported or negative configurations");
    bool rejected=false;try{(void)UICornerRadius::fixed(std::numeric_limits<float>::infinity());}catch(const std::invalid_argument&){rejected=true;}expect(rejected,"reject nonfinite radii");
    rejected=false;try{(void)UICornerRadius::containerConcentric(-1);}catch(const std::invalid_argument&){rejected=true;}expect(rejected,"reject negative minimum");
    rejected=false;try{(void)child->effectiveRadius(static_cast<UIRectCorner>(128));}catch(const std::invalid_argument&){rejected=true;}expect(rejected,"reject invalid corner mask");
}
void platform() {
    auto context=std::make_shared<Context>();SkiaCtx::_main=context;
    auto window=new_shared<UIWindow>();window->setFrame({0,0,300,400});
    auto child=new_shared<UIView>(window->bounds());window->addSubview(child);
    child->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::containerConcentric()));
    radii(child,context->radii,"platform per-corner radii pass through square ancestors");
    context->radii={20,30,40,50};window->layoutIfNeeded();radii(child,context->radii,"platform changes refresh configurations");
    for(size_t i=0;i<4;++i)expect(near(std::any_cast<float>(child->layer()->value(std::string("configuredCornerRadius.")+std::array<const char*,4>{"topLeft","topRight","bottomLeft","bottomRight"}[i]).value()),context->radii[i]),"platform radius reaches retained layer");
    SkiaCtx::_main=nullptr;
}
class SolidDrawingLayer : public CALayer {
public:
    void draw(SkCanvas* canvas)override { canvas->clear(SK_ColorWHITE); }
};
void rendering() {
    for(const auto curve:{CALayerCornerCurve::circular,CALayerCornerCurve::continuous})for(int scale:{1,2,3}) {
        auto view=new_shared<UIView>(NXRect(0,0,100,100));view->setBackgroundColor(UIColor::white);view->layer()->setCornerCurve(curve);
        view->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::fixed(30),UICornerRadius::fixed(0),UICornerRadius::fixed(0),UICornerRadius::fixed(30)));
        auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100*scale,100*scale));surface->getCanvas()->clear(SK_ColorBLACK);surface->getCanvas()->scale(scale,scale);view->layer()->skiaRender(surface->getCanvas());
        SkBitmap pixels;pixels.allocN32Pixels(surface->width(),surface->height());surface->readPixels(pixels,0,0);
        expect(SkColorGetR(pixels.getColor(scale,scale))<10&&SkColorGetR(pixels.getColor(98*scale,98*scale))<10,"rounded diagonal corners cut out background");
        expect(SkColorGetR(pixels.getColor(98*scale,scale))>245&&SkColorGetR(pixels.getColor(scale,98*scale))>245,"independent square corners remain filled");
        expect(SkColorGetR(pixels.getColor(50*scale,50*scale))>245,"uneven shape preserves center");
        view->setClipsToBounds(true);auto fill=new_shared<UIView>(view->bounds());fill->setBackgroundColor(UIColor::white);view->addSubview(fill);
        surface->getCanvas()->clear(SK_ColorBLACK);view->layer()->skiaRender(surface->getCanvas());surface->readPixels(pixels,0,0);
        expect(SkColorGetR(pixels.getColor(scale,scale))<10,"configured corners clip descendants");
        expect(view->layer()->copy()->value("configuredCornerRadius.topLeft").has_value(),"copy retains configured radius");
        auto custom=new_shared<UIView>(NXRect(0,0,100,100),new_shared<SolidDrawingLayer>());
        custom->setCornerConfiguration(view->cornerConfiguration());
        surface->getCanvas()->clear(SK_ColorBLACK);custom->layer()->skiaRender(surface->getCanvas());surface->readPixels(pixels,0,0);
        expect(SkColorGetR(pixels.getColor(scale,scale))<10,"corner configuration rounds custom layer output without clipping descendants");
    }
}
void animation() {
    auto view=new_shared<UIView>(NXRect(0,0,200,200));
    view->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::fixed(10)));
    UIView::animate(10,[&]{view->setCornerConfiguration(UICornerConfiguration::corners(UICornerRadius::fixed(50)));});
    view->layer()->animateAt(Timer());const auto presentation=view->layer()->presentation();expect(bool(presentation),"corner configuration creates implicit animations");
    if(presentation)expect(std::any_cast<float>(*presentation->value("configuredCornerRadius.topLeft"))<20,"presentation starts at previous radius");
    expect(near(view->effectiveRadius(UIRectCorner::allCorners),50),"effective query describes model endpoint during animation");
    UIView::completePendingAnimations();expect(!view->layer()->presentation(),"completion removes corner presentation");
}
}
int main(){resolution();platform();rendering();animation();return failures?1:0;}
