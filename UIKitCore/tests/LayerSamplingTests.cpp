#include <CALayer.h>
#include <CGImage.h>
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include <cmath>
#include <iostream>
#include <limits>
using namespace NXKit;

int main() {
    int failures=0;
    const auto expect=[&](bool pass,const char* message) { if(!pass){++failures;std::cerr<<"FAIL: "<<message<<'\n';} };
    for(const auto transform:{NXAffineTransform::translationBy(13,-7),NXAffineTransform::rotationBy(37),NXAffineTransform(1.2f,.3f,-.4f,.9f,13,-7),NXAffineTransform(-1,0,0,1,7,2)}) {
        const auto inverse=transform.inverted();expect(bool(inverse),"finite nonsingular affine transforms, including reflections, can be inverted");
        if(inverse) for(const NXPoint point: {NXPoint(),NXPoint(4,-9),NXPoint(350,151)}) {
            const auto restored=point.applying(transform).applying(*inverse);
            expect(std::abs(restored.x-point.x)<.0001f&&std::abs(restored.y-point.y)<.0001f,"inverse affine mapping restores translated and transformed contact coordinates");
        }
    }
    expect(!NXAffineTransform(1,2,2,4,0,0).inverted(),"singular transforms reject inversion");
    expect(!NXAffineTransform(1,0,0,1,std::numeric_limits<float>::quiet_NaN(),0).inverted(),"nonfinite transforms reject inversion");
    for(const int scale:{1,2,3}) for(const bool masked:{false,true}) {
        SkBitmap texture;texture.allocN32Pixels(8*scale,8*scale);texture.eraseColor(SK_ColorBLACK);
        for(int y=0;y<8*scale;++y)for(int x=4*scale;x<8*scale;++x)*texture.getAddr32(x,y)=SK_ColorWHITE;
        auto content=new_shared<CALayer>();content->setBounds({0,0,8,8});content->setPosition({4,4});
        content->setContents(std::make_shared<CGImage>(texture.asImage()));content->setContentsScale(scale);
        auto parent=new_shared<CALayer>();parent->setBounds({0,0,8,8});parent->setPosition({4,4});parent->addSublayer(content);
        if(masked) {auto mask=new_shared<CALayer>();mask->setBounds({0,0,8,8});mask->setPosition({4,4});mask->setBackgroundColor(UIColor::white);parent->setMask(mask);}
        const auto render=[&](float phase) {
            parent->setPosition({4+phase/scale,4});
            auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(10*scale,10*scale));
            surface->getCanvas()->clear(SK_ColorBLACK);surface->getCanvas()->scale(scale,scale);parent->skiaRender(surface->getCanvas());
            SkBitmap pixels;pixels.allocN32Pixels(10*scale,10*scale);surface->readPixels(pixels,0,0);return pixels;
        };
        auto aligned=render(0);
        expect(SkColorGetR(aligned.getColor(4*scale-1,4*scale))==0&&SkColorGetR(aligned.getColor(4*scale,4*scale))==255,"aligned backing pixels retain full coverage without an extra blur");
        auto quarter=render(.25f);
        expect(std::abs(int(SkColorGetR(quarter.getColor(4*scale,4*scale)))-191)<=2,"fractional motion interpolates the backing image at each display scale, including inside masks");
        int previous=255;
        for(int step=1;step<=8;++step) {
            auto frame=render(step/8.0f);const int edge=SkColorGetR(frame.getColor(4*scale,4*scale));
            expect(edge<=previous&&previous-edge<40,"image edges move continuously instead of jumping between nearest-neighbor pixels");previous=edge;
        }
    }
    if(failures)return 1;
    std::cout<<"Layer sampling tests passed\n";
}
