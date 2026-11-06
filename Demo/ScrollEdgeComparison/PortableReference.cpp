// Render the same public navigation/tab/scroll configuration as NativeReference.m.
// Input CSV: name,dark,style,x,y,obstruct,manual,hidden,mask,sideChrome,sparse (mask - means no probe).
#include <UIKit.h>
#include <UIApplication.h>
#include <UIApplicationDelegate.h>
#include <UIWindow.h>
#include <private/UIBlurEffect.h>
#include <UIScrollEdgeElementContainerInteraction.h>
#include <UIButton.h>
#include "UIScrollPocket.hpp"
#include "platforms/apple/AppleSystemFont.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkSurface.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace NXKit;
namespace NXKit {
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>){return true;}
class DispatchQueueTestHarness { public: static void pump(){DispatchQueue::main()->performAll();} };
}
namespace ScrollEdgeReference {
class Context final: public SkiaCtx {
public:
    bool dark=false;NXFloat scale=2;
    NXKit::NXSize size={402,874};
    UIEdgeInsets safe={62,0,34,0};
    Context(){fontMgr=SkFontMgr_New_CoreText(nullptr);}
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style=SkFontStyle(),NXFloat size=17)const override{return Internal::appleSystemTypeface(style,size);}
    NXKit::NXSize getSize()override{return size;}
    float getScaleFactor()override{return scale;}
    UIEdgeInsets deviceSafeAreaInsets()override{return safe;}
    UIUserInterfaceStyle getThemeMode()override{return dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;}
    sk_sp<SkSurface> getBackbufferSurface()override{return nullptr;}
    void swapBuffers()override{}void sleepForNextFrame()override{}
    bool platformRunLoop(std::function<bool()>)override{return false;}
protected:
    skgpu::graphite::Context* graphiteContext()override{return nullptr;}
    skgpu::graphite::Recorder* graphiteRecorder()override{return nullptr;}
};
class GridLayer final: public CALayer {
public:
    bool sparse=false;
    explicit GridLayer(bool isSparse=false):sparse(isSparse){}
    explicit GridLayer(GridLayer* source):CALayer(source),sparse(source->sparse){}
    std::shared_ptr<CALayer> copy()override{return new_shared<GridLayer>(this);}
    void draw(SkCanvas* canvas)override {
        const UIColor colors[]={UIColor::systemRed,UIColor::systemBlue,UIColor::systemGreen,UIColor::systemOrange,UIColor::systemPurple};
        const SkFontStyle bold(SkFontStyle::kBold_Weight,SkFontStyle::kNormal_Width,SkFontStyle::kUpright_Slant);
        SkFont font(SkiaCtx::main()->getDefaultTypeface(bold,11),11);SkFontMetrics metrics;font.getMetrics(&metrics);
        SkPaint fill,ink;ink.setAntiAlias(true);ink.setColor(UIColor::label.raw());
        for(int y=0;y<32;++y)for(int x=0;x<18;++x){
            if(sparse&&(x+y)%2==0)continue;
            fill.setColor(colors[(x+y)%5].raw());canvas->drawRect(SkRect::MakeXYWH(x*48,y*48,48,48),fill);
            const auto label=std::to_string(x)+","+std::to_string(y);
            canvas->drawSimpleText(label.data(),label.size(),SkTextEncoding::kUTF8,x*48+3,y*48+14-metrics.fAscent,font,ink);
        }
    }
};
class SideButton final: public UIButton {
public:
    void layoutSubviews()override {UIButton::layoutSubviews();const auto label=titleLabel();label->setAutolayoutEnabled(false);const auto size=label->sizeThatFits(bounds().size);label->setFrame({(bounds().width()-size.width)*.5f,(bounds().height()-size.height)*.5f,size.width,size.height});}
};
class EdgeScreen final: public UIViewController {
public:
    bool obstruct=false,manual=false,sideChrome=false,sparse=false;
    std::vector<std::shared_ptr<UIView>> sideContainers;
    UIEdgeInsets viewportInsets;
    std::shared_ptr<UIScrollView> scroll;
    std::shared_ptr<UIView> host;
    void loadView()override {
        auto root=new_shared<UIView>();root->setBackgroundColor(UIColor::systemBackground);
        if(obstruct)root->addSubview(new_shared<UIView>());
        host=new_shared<UIView>();root->addSubview(host);
        scroll=new_shared<UIScrollView>();host->addSubview(scroll);
        auto grid=new_shared<UIView>(NXRect{0,0,864,1536},new_shared<GridLayer>(sparse));scroll->addSubview(grid);scroll->setContentSize({864,1536});
        setView(root);setAdditionalSafeAreaInsets({0,32,0,32});setTitle("Scroll edges");
        navigationItem()->setRightBarButtonItem(new_shared<UIBarButtonItem>(UIImage::systemImageNamed("ellipsis",17,400,2),UIBarButtonItemStyle::plain));
        if(manual)setContentScrollView(scroll,NSDirectionalRectEdge::all);
        if(sideChrome)installSideChrome();
    }
    void installSideChrome() {
        if(!sideContainers.empty())return;
        for(int side=0;side<2;++side){
            auto container=new_shared<UIView>();view()->addSubview(container);sideContainers.push_back(container);
            auto button=new_shared<SideButton>();button->setText(side?"R":"L");button->titleLabel()->setFontWeight(400);button->setFrame({0,240,32,160});container->addSubview(button);
            auto interaction=std::make_shared<UIScrollEdgeElementContainerInteraction>();interaction->setScrollView(scroll);interaction->setEdge(side?UIRectEdge::right:UIRectEdge::left);container->addInteraction(interaction);
        }
        view()->setNeedsLayout();
    }
    void viewDidLayoutSubviews()override {
        UIViewController::viewDidLayoutSubviews();host->setFrame(view()->bounds());
        const auto size=host->bounds().size;
        scroll->setFrame({viewportInsets.left,viewportInsets.top,size.width-viewportInsets.left-viewportInsets.right,size.height-viewportInsets.top-viewportInsets.bottom});
        for(std::size_t side=0;side<sideContainers.size();++side)sideContainers[side]->setFrame({side?view()->bounds().width()-32:0,0,32,view()->bounds().height()});
    }
};
void settle(const std::shared_ptr<UIWindow>& window){
    for(int i=0;i<5;++i){window->drawAndLayoutTreeIfNeeded();UIView::completePendingAnimations();DispatchQueueTestHarness::pump();}
}
struct Scene {
    std::shared_ptr<UIWindow> window;
    std::shared_ptr<EdgeScreen> screen;
    std::shared_ptr<UINavigationController> navigation;
    std::shared_ptr<UITabBarController> tabs;
    Scene(bool obstruct=false,bool manual=false,bool sideChrome=false,bool sparse=false) {
        screen=new_shared<EdgeScreen>();screen->obstruct=obstruct;screen->manual=manual;screen->sideChrome=sideChrome;screen->sparse=sparse;
        navigation=new_shared<UINavigationController>(screen);
        navigation->tabBarItem()->setTitle("Edges");navigation->tabBarItem()->setImage(UIImage::systemImageNamed("square.grid.2x2",25,400,2));
        tabs=new_shared<UITabBarController>();tabs->setViewControllers({navigation});window=new_shared<UIWindow>();
        UIApplication::shared->delegate->window=window;window->setRootViewController(tabs);window->makeKeyAndVisible();settle(window);
    }
    ~Scene(){window->setRootViewController(nullptr);if(UIApplication::shared){UIApplication::shared->delegate->window.reset();UIApplication::shared->keyWindow.reset();}}
};
std::shared_ptr<UIImage> imageMask(const std::string& mode) {
    if(mode=="nil")return nullptr;
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(2,256));surface->getCanvas()->clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    for(int y=0;y<256;++y){const auto alpha=mode=="zero"?0:mode=="ramp"?y:255;paint.setColor(SkColorSetARGB(alpha,255,255,255));surface->getCanvas()->drawRect(SkRect::MakeXYWH(0,y,2,1),paint);}
    return std::make_shared<UIImage>(std::make_shared<NXKit::CGImage>(surface->makeImageSnapshot()),1);
}
SkBitmap render(const std::shared_ptr<UIWindow>& window,NXFloat scale=2,bool finishAnimations=true) {
    if(finishAnimations)settle(window);const int width=std::lround(402*scale),height=std::lround(874*scale);
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width,height));auto canvas=surface->getCanvas();canvas->clear(SK_ColorTRANSPARENT);canvas->scale(scale,scale);
    UITraitCollection::setCurrent(window->traitCollection());window->layer()->presentationOrSelf()->skiaRender(canvas);
    SkBitmap pixels;pixels.allocN32Pixels(width,height);surface->readPixels(pixels,0,0);DispatchQueueTestHarness::pump();return pixels;
}
NXRect bandRect(const Internal::ScrollPocketSnapshot& state,int edge) {
    if(!state.visible)return {};
    auto result=state.viewport;
    switch(edge){case 0:result.size.height=state.effectExtent;break;case 1:result.size.width=state.effectExtent;break;case 2:result.origin.y=result.maxY()-state.effectExtent;result.size.height=state.effectExtent;break;default:result.origin.x=result.maxX()-state.effectExtent;result.size.width=state.effectExtent;}
    return result;
}
void rect(std::ostream& out,NXRect r){out<<'['<<r.minX()<<','<<r.minY()<<','<<r.width()<<','<<r.height()<<']';}
void insets(std::ostream& out,UIEdgeInsets i){out<<'['<<i.top<<','<<i.left<<','<<i.bottom<<','<<i.right<<']';}
}
#ifndef NXKIT_SCROLL_EDGE_REFERENCE_NO_MAIN
int main(int argc,char** argv) {
    using namespace ScrollEdgeReference;if(argc!=3)return 2;
    const std::filesystem::path directory=argv[2];std::filesystem::create_directories(directory);
    auto context=std::make_shared<Context>();SkiaCtx::_main=context;
    UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
    std::ifstream input(argv[1]);if(!input)return 2;std::ofstream trace(directory/"states.json");trace<<'[';std::string line;bool comma=false;int count=0;
    while(std::getline(input,line)) {
        std::replace(line.begin(),line.end(),',',' ');std::istringstream row(line);std::string name,style,mask;int dark,obstruct,manual,hidden;NXFloat x,y;
        row>>name>>dark>>style>>x>>y>>obstruct>>manual>>hidden>>mask;if(!row)return 3;int sideChrome=0,sparse=0;row>>sideChrome>>sparse;
        context->dark=dark;Scene scene(obstruct,manual,sideChrome,sparse);auto scroll=scene.screen->scroll;
        const auto value=style=="none"?UIScrollEdgeEffectStyle::none:style=="hard"?UIScrollEdgeEffectStyle::hard:style=="soft"?UIScrollEdgeEffectStyle::soft:UIScrollEdgeEffectStyle::automatic;
        for(auto edge:{scroll->topEdgeEffect(),scroll->leftEdgeEffect(),scroll->bottomEdgeEffect(),scroll->rightEdgeEffect()}){edge->setStyle(value);edge->setHidden(hidden);}
        scroll->setContentOffset({x,y},false);settle(scene.window);
        std::string probeException;
        if(mask!="-")try{auto probe=new_shared<BackdropEffectView>(UIBlurEffect::effectWithVariableBlurRadius(24,imageMask(mask)));probe->setFrame({24,220,354,256});scene.window->addSubview(probe);}catch(...){if(mask!="nil")throw;probeException="UIBlurEffect rejected nil imageMask";}
        const auto pixels=render(scene.window);SkFILEWStream file((directory/(name+".png")).c_str());if(!SkPngEncoder::Encode(&file,pixels.pixmap(),{}))return 4;
        if(comma)trace<<',';comma=true;++count;trace<<"{\"sparse\":"<<sparse<<",\"sideChrome\":"<<sideChrome<<",\"maskProbe\":"<<std::quoted(mask)<<",\"probeException\":"<<std::quoted(probeException)<<",\"name\":"<<std::quoted(name)<<",\"style\":"<<std::quoted(style)<<",\"dark\":"<<dark<<",\"offset\":["<<scroll->contentOffset().x<<','<<scroll->contentOffset().y<<"],\"safe\":";insets(trace,scroll->safeAreaInsets());trace<<",\"adjustedInset\":";insets(trace,scroll->adjustedContentInset());trace<<",\"bounds\":";rect(trace,scroll->bounds());
        trace<<",\"getterTop\":"<<(scene.screen->contentScrollViewForEdge(NSDirectionalRectEdge::top)==scroll)<<",\"edges\":[";
        const auto host=scene.screen->view()->superview().lock();
        for(int edge=0;edge<4;++edge){if(edge)trace<<',';const auto state=Internal::scrollPocketSnapshot(*scene.screen,host,edge);trace<<"{\"visible\":"<<state.visible<<",\"strength\":"<<state.strength<<",\"safeExtent\":"<<state.safeExtent<<",\"effectExtent\":"<<state.effectExtent<<",\"bandRect\":";rect(trace,bandRect(state,edge));trace<<",\"bandRectSource\":\"renderer snapshot extent\",\"viewport\":";rect(trace,state.viewport);trace<<'}';}trace<<"]}";
    }
    trace<<"]\n";std::cout<<"Rendered "<<count<<" scroll edge states\n";UIApplication::shared.reset();SkiaCtx::_main.reset();return 0;
}
#endif
