#include "../app/Screens/GlassDemoViewController/GlassDemoScene.hpp"
#include <DispatchQueue.h>
#include "include/core/SkSurface.h"
#include "include/core/SkBitmap.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>

using namespace NXKit;
namespace NXKit {
class GlassAppearanceTestHarness {
public:
    static std::shared_ptr<UITraitCollection> traits(bool dark) {
        auto traits=std::make_shared<UITraitCollection>();
        traits->_userInterfaceStyle=dark ? UIUserInterfaceStyle::dark : UIUserInterfaceStyle::light;
        traits->_displayScale=2; return traits;
    }
};
class DispatchQueueTestHarness { public: static void pump() { DispatchQueue::main()->performAll(); } };
}
class GalleryRoot : public UIView {
public:
    explicit GalleryRoot(bool dark):UIView(NXRect(0,0,720,440)) { _traitCollection=GlassAppearanceTestHarness::traits(dark); }
};
void render(const std::shared_ptr<GalleryRoot>& root, const std::shared_ptr<GlassDemoScene>& scene,
            const std::string& path, int dark) {
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1440,880));
    auto* canvas=surface->getCanvas(); canvas->scale(2,2);
    const int passes=scene->demo()==GlassDemoContrast26 ? 2 : 1;
    for (int pass=0;pass<passes;++pass) { canvas->clear(SK_ColorBLACK); root->layer()->skiaRender(canvas); DispatchQueueTestHarness::pump(); }
    SkBitmap pixels; pixels.allocN32Pixels(1440,880); if (!surface->readPixels(pixels,0,0)) throw std::runtime_error("Cannot read gallery pixels");
    std::ofstream output(path,std::ios::binary); output<<"P6\n1440 880\n255\n";
    for (int y=0;y<880;++y) for (int x=0;x<1440;++x) {
        auto c=pixels.getColor(x,y); const unsigned char rgb[]={static_cast<unsigned char>(SkColorGetR(c)),static_cast<unsigned char>(SkColorGetG(c)),static_cast<unsigned char>(SkColorGetB(c))};
        output.write(reinterpret_cast<const char*>(rgb),3);
    }
    std::ofstream metadata(path+".json");
    metadata<<"{\"case\":"<<scene->demo()<<",\"dark\":"<<dark<<",\"progress\":"<<scene->progress()<<",\"backdrop\":"<<scene->backdrop()<<",\"elements\":[";
    for (size_t i=0;i<scene->glassItems().size();++i) {
        const auto& item=scene->glassItems()[i]; const auto frame=item->frame();
        if (i) metadata<<',';
        metadata<<"{\"x\":"<<frame.origin.x<<",\"y\":"<<frame.origin.y<<",\"width\":"<<frame.width()<<",\"height\":"<<frame.height()<<",\"radius\":"<<item->layer()->cornerRadius()<<'}';
    }
    metadata<<"]}\n";
}
int main(int argc,char** argv) {
    if (argc!=7 && argc!=4) {
        std::cerr<<"Usage: GalleryRenderer ARTWORK OUTPUT.ppm CASE DARK PROGRESS BACKDROP\n"
                   "   or: GalleryRenderer ARTWORK OUTPUT_DIRECTORY MANIFEST\n"; return 2;
    }
    auto image=UIImage::fromPath(argv[1],1); if (!image || image->size()!=NXSize(720,440)) return 2;
    std::shared_ptr<GalleryRoot> root;
    std::shared_ptr<GlassDemoScene> scene;
    int currentDemo=-1,currentDark=-1;
    const auto draw=[&](const std::string& path,int demo,int dark,float progress,int backdrop,int reverse) {
        if (demo<0 || demo>=GlassDemoCount) throw std::invalid_argument("Invalid demo");
        if (demo!=currentDemo || dark!=currentDark) {
            root=new_shared<GalleryRoot>(dark); scene=new_shared<GlassDemoScene>(static_cast<GlassDemoID>(demo),image);
            root->addSubview(scene); currentDemo=demo; currentDark=dark;
        }
        scene->setReversedOrder(reverse); scene->setProgress(progress); scene->setBackdrop(backdrop);
        if (std::getenv("NXKIT_GLASS_PROBE")) {
            // Calibration-only raw transmission, independent of the material's
            // color grading. No probe override is part of the app scene.
            for (const auto& item:scene->glassItems()) {
                auto effect=*item->glassEffect(); effect.setTintOpacity(0); effect.setSaturation(1);
                effect.setContrast(1); effect.setLuminosity(0); effect.setBrightBackdropShade(0);
                effect.setEdgeStrength(0); effect.setEdgeAbsorption(0); item->setEffect(effect);
            }
        }
        render(root,scene,path,dark);
    };
    if (argc==7) draw(argv[2],std::stoi(argv[3]),std::stoi(argv[4]),std::stof(argv[5]),std::stoi(argv[6]),0);
    else {
        std::ifstream input(argv[3]); std::string line;
        while (std::getline(input,line)) {
            std::istringstream fields(line); std::string name; int demo,dark,backdrop,reverse=0; float progress;
            if (!(fields>>std::quoted(name)>>demo>>dark>>progress>>backdrop)) return 2;
            fields>>reverse;
            draw(std::string(argv[2])+"/"+name,demo,dark,progress,backdrop,reverse);
            std::cout<<name<<std::endl;
        }
    }
}
