// Retained production rendering of the independent native scene manifest.
#include <BackdropEffectView.h>
#include <UIImageView.h>
#include "include/core/SkSurface.h"
#include "include/core/SkBitmap.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
using namespace NXKit;
namespace NXKit {
class GlassMaterialTestHarness {
public:
 static auto traits(bool dark,float amount) {auto t=UITraitCollection::traitCollectionWithGlassTintAmount(amount);t->_userInterfaceStyle=dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;t->_displayScale=3;return t;}
};
}
class MatrixRoot:public UIView {
public: MatrixRoot(bool dark,float amount):UIView({0,0,384,704}){_traitCollection=GlassMaterialTestHarness::traits(dark,amount);layer()->setContentsScale(3);}
};
int main(int argc,char **argv) {
 if(argc<2 || argc>3)return 2;std::ifstream manifest(argv[1]);std::string line;
 while(std::getline(manifest,line)) {
  std::istringstream input(line);std::string output,background;int dark;float amount;
  if(!(input>>std::quoted(output)>>std::quoted(background)>>dark>>amount))return 2;
  auto root=new_shared<MatrixRoot>(dark,amount);auto bg=new_shared<UIImageView>();bg->setImage(UIImage::fromPath(background,2));bg->setFrame(root->bounds());root->addSubview(bg);
  float x,y,w,h;int clear,tinted;
  while(input>>x>>y>>w>>h>>clear>>tinted) {
   UIGlassEffect effect(clear?UIGlassEffect::Style::clear:UIGlassEffect::Style::regular);if(tinted)effect.setTintColor(UIColor(0,122,255));
   if(argc==3)effect.setFrostRadius(std::stof(argv[2]));
   auto glass=new_shared<BackdropEffectView>(effect);glass->setFrame({x,y,w,h});glass->layer()->setCornerRadius(h/2);glass->layer()->setContentsScale(3);root->addSubview(glass);
  }
  auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1152,2112));surface->getCanvas()->scale(3,3);surface->getCanvas()->clear(SK_ColorBLACK);root->layer()->skiaRender(surface->getCanvas());
  SkBitmap pixels;pixels.allocN32Pixels(1152,2112);if(!surface->readPixels(pixels,0,0))return 1;
  std::ofstream file(output,std::ios::binary);file<<"P6\n1152 2112\n255\n";
  for(int yy=0;yy<2112;yy++)for(int xx=0;xx<1152;xx++){auto c=pixels.getColor(xx,yy);const unsigned char rgb[]={static_cast<unsigned char>(SkColorGetR(c)),static_cast<unsigned char>(SkColorGetG(c)),static_cast<unsigned char>(SkColorGetB(c))};file.write(reinterpret_cast<const char*>(rgb),3);}
  std::cout<<output<<'\n';
 }
}
