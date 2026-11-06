// Optical checkpoints driven by measured native presentation rectangles.
// This deliberately bypasses NXKit's touch dynamics; it is not a motion test.
#include <GlassContainerView.h>
#include <BackdropEffectView.h>
#include <UIImageView.h>
#include "CABackdropEffectLayer.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
using namespace NXKit;
namespace NXKit {
class GlassContainerTestHarness {
public:
    static std::shared_ptr<UITraitCollection> traits(bool dark) {
        auto traits=std::make_shared<UITraitCollection>();
        traits->_userInterfaceStyle=dark ? UIUserInterfaceStyle::dark : UIUserInterfaceStyle::light;
        traits->_displayScale=2; return traits;
    }
};
}
namespace {
struct Checkpoint {
    std::string path,artwork;
    bool dark=false,pair=true,reverse=false,clear=false;
    NXFloat glow=0,spacing=8;
    NXPoint touch;
    NXRect frames[2];
    UIColor background=UIColor(128,128,128);
    std::array<std::optional<UIColor>,2> tints;
};
class Root final: public UIView {
public:
    explicit Root(const Checkpoint& state): UIView({0,0,720,440}) {
        _traitCollection=GlassContainerTestHarness::traits(state.dark);
        setBackgroundColor(state.background); layer()->setContentsScale(2);
        if (!state.artwork.empty()) {
            auto image=UIImage::fromPath(state.artwork,1);
            if (!image) throw std::runtime_error("Cannot load checkpoint artwork");
            auto view=new_shared<UIImageView>(image); view->setFrame(bounds());
            view->setContentMode(UIViewContentMode::scaleToFill); addSubview(view);
        }
    }
};
void render(const Checkpoint& state) {
    auto root=new_shared<Root>(state);
    UIGlassContainerEffect grouping; grouping.setSpacing(state.spacing);
    auto group=new_shared<GlassContainerView>(grouping); group->setFrame(root->bounds());
    std::shared_ptr<UIView> host=root;
    if (state.pair) { root->addSubview(group); host=group->contentView(); }
    // Freeze the native 2x backing resolution in both old and current archives;
    // older builds did not yet inherit displayScale into effect layers.
    group->layer()->setContentsScale(2);
    const int count=state.pair ? 2 : 1;
    for (int slot=0;slot<count;++slot) {
        const int i=state.reverse ? count-1-slot : slot;
        const bool target=i==(state.pair ? 1 : 0);
        UIGlassEffect material(state.clear ? UIGlassEffect::Style::clear : UIGlassEffect::Style::regular);
        if (state.tints[i]) material.setTintColor(*state.tints[i]);
        auto item=new_shared<BackdropEffectView>(material);
        const auto f=state.frames[i]; const auto scaleX=f.width()/52,scaleY=f.height()/52;
        item->setFrame({f.midX()-26,f.midY()-26,52,52}); item->layer()->setCornerRadius(26);
        host->addSubview(item); item->setTransform(NXAffineTransform(scaleX,0,0,scaleY,0,0));
        item->layer()->setContentsScale(2);
        if (target) {
            Internal::GlassInteraction lighting; lighting.glowProgress=state.glow;
            lighting.location={(state.touch.x-f.minX())/scaleX,(state.touch.y-f.minY())/scaleY};
            std::static_pointer_cast<CABackdropEffectLayer>(item->layer())->setGlassInteraction(lighting);
        }
        for (const auto& frame: {NXRect(16,25,20,2),NXRect(25,16,2,20)}) {
            auto stroke=new_shared<UIView>(frame);
            stroke->setBackgroundColor(state.tints[i] ? UIColor::white : (state.dark ? UIColor::white : UIColor::black));
            item->addSubview(stroke);
        }
    }
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1440,880));
    surface->getCanvas()->scale(2,2); surface->getCanvas()->clear(SK_ColorBLACK);
    root->layer()->skiaRender(surface->getCanvas());
    SkBitmap pixels; pixels.allocN32Pixels(1440,880);
    if (!surface->readPixels(pixels,0,0)) throw std::runtime_error("Cannot read checkpoint");
    std::ofstream file(state.path,std::ios::binary); file<<"P6\n1440 880\n255\n";
    for (int y=0;y<880;++y) for (int x=0;x<1440;++x) {
        const auto c=pixels.getColor(x,y);
        const unsigned char rgb[]={static_cast<unsigned char>(SkColorGetR(c)),static_cast<unsigned char>(SkColorGetG(c)),static_cast<unsigned char>(SkColorGetB(c))};
        file.write(reinterpret_cast<const char*>(rgb),3);
    }
}
UIColor rgb(int r,int g,int b,int a=255) {
    return UIColor(static_cast<unsigned char>(r),static_cast<unsigned char>(g),static_cast<unsigned char>(b),static_cast<unsigned char>(a));
}
bool read(std::istringstream& fields,Checkpoint& state) {
    float ax,ay,aw,ah,bx,by,bw,bh;
    if (!(fields>>std::quoted(state.path)>>state.dark>>state.glow>>state.touch.x>>state.touch.y>>ax>>ay>>aw>>ah>>bx>>by>>bw>>bh)) return false;
    state.frames[0]={ax,ay,aw,ah};state.frames[1]={bx,by,bw,bh};return true;
}
}
int main(int argc,char** argv) {
    if (argc!=2) { std::cerr<<"Usage: MergeCheckpointRenderer MANIFEST\n"; return 2; }
    std::ifstream input(argv[1]); std::string line;
    while (std::getline(input,line)) {
        Checkpoint state; const bool version2=line.starts_with("v2 ");
        std::istringstream fields(version2 ? line.substr(3) : line);
        if (!read(fields,state)) return 2;
        if (version2) {
            int r,g,b;
            if (!(fields>>state.pair>>state.reverse>>state.spacing>>state.clear>>r>>g>>b>>std::quoted(state.artwork))) return 2;
            state.background=rgb(r,g,b);
            for (int i=0;i<2;++i) {
                bool hasTint;int a;
                if (!(fields>>hasTint>>r>>g>>b>>a)) return 2;
                if (hasTint) state.tints[i]=rgb(r,g,b,a);
            }
        } else {
            int tinted=1,r=0,g=122,b=255;fields>>state.pair>>tinted>>r>>g>>b;
            if (tinted) state.tints[state.pair ? 1 : 0]=rgb(r,g,b);
        }
        render(state); std::cout<<state.path<<std::endl;
    }
}
