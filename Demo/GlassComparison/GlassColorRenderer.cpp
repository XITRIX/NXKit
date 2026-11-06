// Transfer-only regression: measured native material pixels are supplied as
// input, independently of NXKit's optical/material reconstruction.
#include <BackdropEffectView.h>
#include <LensView.h>
#include "include/core/SkSurface.h"
#include "include/core/SkBitmap.h"
#include <fstream>
#include <iostream>
#include <sstream>
using namespace NXKit;
namespace NXKit {
class GlassMaterialTestHarness {
public:
 static auto traits(bool dark,float scale) {auto t=std::make_shared<UITraitCollection>();t->_displayScale=scale;t->_userInterfaceStyle=dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;return t;}
};
class ColorRoot final:public UIView {
public:
 ColorRoot(bool dark,float scale) {_traitCollection=GlassMaterialTestHarness::traits(dark,scale);setFrame({0,0,240,96});}
};
}
int main(int argc,char**argv) {
 if(argc!=2)return 2;std::ifstream file(argv[1]);std::string line;
 while(std::getline(file,line)) {
  std::istringstream in(line);std::string id;int dark,sr,sg,sb,br,bg,bb;float alpha,scale;
  in>>id>>dark>>sr>>sg>>sb>>br>>bg>>bb>>alpha>>scale;if(!in||scale<1||scale>3)return 2;
  auto root=new_shared<ColorRoot>(dark,scale);root->setBackgroundColor(UIColor(br,bg,bb));
  UIGlassEffect effect(UIGlassEffect::Style::clear);effect.setFrostRadius(0);effect.setRefraction(0);effect.setSaturation(1);effect.setContrast(1);effect.setLuminosity(0);effect.setTintOpacity(0);effect.setEdgeStrength(0);effect.setSpecularStrength(0);effect.setContentVibrancyEnabled(true);
  auto glass=new_shared<BackdropEffectView>(effect);glass->setFrame({10,10,100,64});root->addSubview(glass);
  auto lens=new_shared<LensView>(LensView::Style::selection);lens->setFrame({130,10,100,64});lens->setContentVibrancyEnabled(true);root->addSubview(lens);
  for(auto host:{std::static_pointer_cast<UIView>(glass),lens->contentView()}) {auto ink=new_shared<UIView>(NXRect(30,20,24,20));ink->setBackgroundColor(UIColor(sr,sg,sb));ink->setAlpha(alpha);host->addSubview(ink);}
  auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(240*scale,96*scale));surface->getCanvas()->scale(scale,scale);surface->getCanvas()->clear(SK_ColorTRANSPARENT);root->layer()->skiaRender(surface->getCanvas());SkBitmap pixels;pixels.allocN32Pixels(240*scale,96*scale);surface->readPixels(pixels,0,0);
  std::cout<<id;
  for(int x:{50,170}) {auto color=pixels.getColor(x*scale,40*scale);std::cout<<' '<<SkColorGetR(color)<<' '<<SkColorGetG(color)<<' '<<SkColorGetB(color);}
  std::cout<<'\n';
 }
}
