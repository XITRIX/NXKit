// Retained rendering counterpart of the independent coordinate-coded UIKit probe.
#include <BackdropEffectView.h>
#include <UIImageView.h>
#include "NativeGlassShader.hpp"
#include "NativeGlassProfile.hpp"
#include "include/core/SkSurface.h"
#include "include/core/SkBitmap.h"
#include <fstream>
#include <iostream>
using namespace NXKit;
using namespace NXKit::Internal;
namespace NXKit {
class GlassMaterialTestHarness {public:static auto traits(){auto t=UITraitCollection::traitCollectionWithGlassTintAmount(.5f);t->_userInterfaceStyle=UIUserInterfaceStyle::dark;t->_displayScale=3;return t;}};
}
class Root:public UIView {public:Root():UIView({0,0,402,874}){_traitCollection=GlassMaterialTestHarness::traits();}};
int main(int argc,char**argv){
 if(argc!=4)return 2;std::string mode=argv[3];auto root=new_shared<Root>();auto bg=new_shared<UIImageView>();bg->setImage(UIImage::fromPath(argv[1],3));bg->setFrame(root->bounds());root->addSubview(bg);
 NXRect frames[]={{30,100,64,64},{150,100,120,120},{30,300,280,64},{30,460,64,160},{150,460,160,160}};
 for(auto f:frames){auto p=nativeGlassProfile(UIGlassEffect::Style::regular,UIUserInterfaceStyle::dark,.5f,f.size,false);p.faceBlack=0;p.faceWhite=1;p.faceSaturation=1;p.faceColorMatrixMaxLuma=p.faceColorMatrixMaxLumaSDR=1;p.faceColorMatrixFillColor={0,0,0,0};p.faceOpacity=1;p.blurRadius=p.blurFillRadius=p.bleedBlurRadius=p.shadowBlurRadius=0;p.blurFillNormalOpacity=p.blurFillLightenOpacity=p.blurFillDarkenOpacity=0;p.blurOpacity0=p.blurOpacity1=p.blurOpacity2=p.blurOpacity3=0;p.refractionOpacity=0;p.bleedOpacity=p.shadowOpacity=p.sdrShadowOpacity=p.ringShadowOpacity=p.keyFillHighlightAmount=p.highlight.keyAmount=p.highlight.fillAmount=0;
 if(mode=="identity")p.innerRefractionAmount=p.outerRefractionAmount=0;
 if(mode=="inner"||mode=="inner-small"||mode=="inner-height")p.outerRefractionAmount=0;
 if(mode=="outer"||mode=="outer-height")p.innerRefractionAmount=0;
 if(mode=="inner-small")p.innerRefractionAmount=-8;
 if(mode=="inner-height")p.innerRefractionHeight=8;
 if(mode=="outer-height")p.outerRefractionHeight=8;
 auto effect=makeNativeGlassEffect(p,UIGlassEffect::Style::regular,UIColor::clear,false,f.width()/2);auto v=new_shared<BackdropEffectView>(effect);v->setFrame(f);v->layer()->setCornerRadius(std::min(f.width(),f.height())/2);v->layer()->setContentsScale(3);root->addSubview(v);}
 auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1206,2622));surface->getCanvas()->clear(SK_ColorBLACK);surface->getCanvas()->scale(3,3);root->layer()->skiaRender(surface->getCanvas());SkBitmap b;b.allocN32Pixels(1206,2622);if(!surface->readPixels(b,0,0))return 1;std::ofstream out(argv[2],std::ios::binary);out<<"P6\n1206 2622\n255\n";for(int y=0;y<2622;y++)for(int x=0;x<1206;x++){auto c=b.getColor(x,y);unsigned char rgb[]={static_cast<unsigned char>(SkColorGetR(c)),static_cast<unsigned char>(SkColorGetG(c)),static_cast<unsigned char>(SkColorGetB(c))};out.write(reinterpret_cast<char*>(rgb),3);}
}
