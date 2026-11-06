// Independent retained comparison composition. No image registration or fitted
// colors: frames and source pixels are supplied by the native research fixture.
#include <BackdropEffectView.h>
#include <LensView.h>
#include <UIImageView.h>
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSurface.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
using namespace NXKit;
namespace NXKit {
class GlassMaterialTestHarness {
public:
    static auto traits(bool dark,int scale) { auto t=std::make_shared<UITraitCollection>(); t->_displayScale=scale; t->_userInterfaceStyle=dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light; return t; }
};
class RefractionRoot final : public UIView {
public:
    explicit RefractionRoot(bool dark,int scale) { _traitCollection=GlassMaterialTestHarness::traits(dark,scale); setFrame({0,0,720,560}); }
};
}
namespace {
class RowMask final : public CALayer {
public:
    NXRect lensFrame;
    bool inverse=true;
    RowMask()=default;
    explicit RowMask(RowMask* old):CALayer(old),lensFrame(old->lensFrame),inverse(old->inverse) {}
    std::shared_ptr<CALayer> copy() override { return new_shared<RowMask>(this); }
    void draw(SkCanvas* canvas) override {
        SkPathBuilder path; path.setFillType(SkPathFillType::kEvenOdd);
        if (inverse) path.addRect(SkRect::MakeWH(bounds().width(),bounds().height()));
        auto f=lensFrame; float r=std::min(f.width(),f.height())*.5f;
        path.addRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(f.minX(),f.minY(),f.width(),f.height()),r,r));
        SkPaint p; p.setColor(SK_ColorWHITE);p.setAntiAlias(true);canvas->drawPath(path.detach(),p);
    }
};
auto image(const std::string& path,NXRect frame,float scale) {
    auto v=new_shared<UIImageView>(UIImage::fromPath(path,scale));v->setFrame(frame);v->setUserInteractionEnabled(false);return v;
}
void write(const std::shared_ptr<UIView>& root,const std::string& output,int scale) {
    const int width=720*scale,height=560*scale;
    auto s=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width,height));s->getCanvas()->scale(scale,scale);s->getCanvas()->clear(SK_ColorBLACK);root->layer()->skiaRender(s->getCanvas());
    SkBitmap b;b.allocN32Pixels(width,height);if(!s->readPixels(b,0,0))throw std::runtime_error("Cannot read raster pixels");
    std::ofstream f(output,std::ios::binary);f<<"P6\n"<<width<<' '<<height<<"\n255\n";
    for(int y=0;y<height;y++)for(int x=0;x<width;x++){auto c=b.getColor(x,y);unsigned char rgb[]={static_cast<unsigned char>(SkColorGetR(c)),static_cast<unsigned char>(SkColorGetG(c)),static_cast<unsigned char>(SkColorGetB(c))};f.write(reinterpret_cast<char*>(rgb),3);}
}
}
int main(int argc,char**argv) {
    if(argc!=2&&argc!=4)return 2;
    const int scale=argc==4&&std::string(argv[2])=="--scale"?std::stoi(argv[3]):2;
    if(scale<1||scale>3)return 2;
    std::ifstream manifest(argv[1]);std::string line;
    while(std::getline(manifest,line)) {
        std::istringstream f(line);std::string output,bg,normal,selected,setup;int dark,style,lifted,mode,warps,platter,hideLens;float x,y,w,h,sx,sy,sw,sh,sourceHostAlpha;
        f>>std::quoted(output)>>std::quoted(bg)>>std::quoted(normal)>>std::quoted(selected)>>dark>>style>>lifted>>std::quoted(setup)>>mode>>warps>>platter>>hideLens>>x>>y>>w>>h>>sx>>sy>>sw>>sh>>sourceHostAlpha;if(!f)return 2;
        auto root=new_shared<RefractionRoot>(dark,scale);root->addSubview(image(bg,root->bounds(),1));
        std::shared_ptr<UIView> host;
        if(platter){UIGlassEffect effect(platter==2?UIGlassEffect::Style::clear:UIGlassEffect::Style::regular);auto glass=new_shared<BackdropEffectView>(effect);glass->setFrame({40,250,640,76});glass->layer()->setCornerRadius(38);root->addSubview(glass);host=glass;}
        else {host=new_shared<UIView>(NXRect(40,250,640,76));root->addSubview(host);}
        auto lens=new_shared<LensView>(style?LensView::Style::selection:LensView::Style::regular);NXRect local(x-40,y-250,w,h);lens->setFrame(local);lens->setRestingBackgroundColor(UIColor(0,0,0,26));
        auto ordinary=image(normal,{sx,sy,sw,sh},scale);auto chosen=image(selected,{sx,sy,sw,sh},scale);
        const bool portal=setup=="portal"||setup=="portal-only";
        if(setup=="portal"||setup=="underlay"||setup=="source-normal") {
            host->addSubview(ordinary);
            if(portal){auto mask=new_shared<RowMask>();mask->lensFrame={local.minX()-sx,local.minY()-sy,w,h};auto view=new_shared<UIView>(NXRect(0,0,sw,sh),mask);ordinary->setMask(view);}
        }
        if(setup=="source-selected")host->addSubview(chosen);
        if(portal) {
#ifdef NXKIT_LENS_REFRACTION_BEFORE
            chosen->setFrame({sx-local.minX(),sy-local.minY(),sw,sh});lens->contentView()->addSubview(chosen);
#else
            // LensView owns suppression of its weak source's ordinary pass.
            // Real caller visibility remains meaningful to source sampling.
            auto sourceHost=new_shared<UIView>(NXRect(0,0,640,76));sourceHost->setAlpha(sourceHostAlpha);sourceHost->addSubview(chosen);host->addSubview(sourceHost);lens->setLiftedContentMode(LensView::LiftedContentMode::hideSource);lens->setLiftedContentView(chosen);
#endif
        }
        host->addSubview(lens);lens->setLifted(lifted);lens->setHidden(hideLens||setup=="source-selected"||setup=="source-normal");host->layoutIfNeeded();write(root,output,scale);std::cout<<output<<'\n';
    }
}
