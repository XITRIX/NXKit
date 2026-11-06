// Retained production LensView + the live Demo tab composition. All frame input
// comes from the native fixture; source typography is held constant via assets.
#include "../app/Screens/GlassDemoViewController/LensTabBarView.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>
using namespace NXKit;
namespace NXKit {
class GlassMaterialTestHarness {
public:
    static std::shared_ptr<UITraitCollection> traits(bool dark) { auto t=std::make_shared<UITraitCollection>(); t->_displayScale=2; t->_userInterfaceStyle=dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light; return t; }
};
class LensRendererRoot final : public UIView {
public:
    LensRendererRoot(bool dark) { _traitCollection=GlassMaterialTestHarness::traits(dark); setFrame({0,0,720,560}); }
};
}
int main(int argc, char** argv) {
    if (argc!=2) return 2;
    std::ifstream manifest(argv[1]); std::string row;
    while (std::getline(manifest,row)) {
        std::istringstream fields(row); std::string output,background,normal,selected; int dark,style,lifted,index;
        fields>>std::quoted(output)>>std::quoted(background)>>std::quoted(normal)>>std::quoted(selected)>>dark>>style>>lifted>>index;
        NXFloat x,y,w,h; fields>>x>>y>>w>>h; if (!fields) return 2;
        auto root=new_shared<LensRendererRoot>(dark); auto image=new_shared<UIImageView>(UIImage::fromPath(background,1)); image->setFrame(root->bounds()); root->addSubview(image);
        auto bar=new_shared<LensTabBarView>(); bar->setFrame({40,250,640,76});
        bar->setComparisonContent(UIImage::fromPath(normal,2),UIImage::fromPath(selected,2));
        bar->setSelectedIndex(index); bar->lens()->setRestingBackgroundColor(UIColor(0,0,0,26)); bar->lens()->setStyle(style?LensView::Style::selection:LensView::Style::regular);
        bar->setLifted(lifted); bar->setComparisonGeometry({x-40,y-250,w,h}); root->addSubview(bar); bar->layoutIfNeeded();
        for (int i=0;i<3;i++) {
            fields>>x>>y>>w>>h; if (!fields) return 2;
            auto lens=new_shared<LensView>(style?LensView::Style::selection:LensView::Style::regular);
            lens->setRestingBackgroundColor(UIColor(0,0,0,26)); lens->setFrame({x,y,w,h}); lens->setLifted(lifted); root->addSubview(lens);
        }
        auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1440,1120)); surface->getCanvas()->scale(2,2); surface->getCanvas()->clear(SK_ColorBLACK); root->layer()->skiaRender(surface->getCanvas());
        SkBitmap pixels; pixels.allocN32Pixels(1440,1120); if (!surface->readPixels(pixels,0,0)) return 1;
        std::ofstream file(output,std::ios::binary); file<<"P6\n1440 1120\n255\n";
        for (int yy=0;yy<1120;yy++) for (int xx=0;xx<1440;xx++) { const auto c=pixels.getColor(xx,yy); const unsigned char rgb[]={static_cast<unsigned char>(SkColorGetR(c)),static_cast<unsigned char>(SkColorGetG(c)),static_cast<unsigned char>(SkColorGetB(c))}; file.write(reinterpret_cast<const char*>(rgb),3); }
        std::cout<<output<<'\n';
    }
}
