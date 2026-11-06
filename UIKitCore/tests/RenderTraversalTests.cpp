#include <CALayer.h>
#include "RenderPerformance.hpp"
#include "LayerContentMask.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPicture.h"
#include "include/effects/SkGradient.h"
#include "include/effects/SkImageFilters.h"
#include <iostream>
#include <vector>

using namespace NXKit;
namespace {
int failures = 0;
void expect(bool condition, const char* message) {
    if (!condition) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
class Probe final : public CALayer {
public:
    int draws = 0, id = 0;
    std::vector<int>* order = nullptr;
    std::function<void()> callback;
    void draw(SkCanvas*) override {
        ++draws;
        if (order) order->push_back(id);
        if (callback) callback();
    }
};
void clipping() {
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(64,64));
    auto* canvas = surface->getCanvas();
    auto parent = new_shared<Probe>(); parent->setFrame({100,0,32,32});
    auto child = new_shared<Probe>(); child->setFrame({-90,8,12,12}); child->setBackgroundColor(UIColor::red);
    parent->addSublayer(child);
    Internal::RenderPerformance stats; Internal::RenderPerformance::current = &stats;
    canvas->clear(SK_ColorWHITE);
    const auto saves = canvas->getSaveCount();
    parent->skiaRender(canvas);
    expect(child->draws==1,"unclipped offscreen parent retains visible overflowing child");
    SkBitmap pixels; pixels.allocN32Pixels(64,64); surface->readPixels(pixels,0,0);
    expect(pixels.getColor(15,12)==SK_ColorRED,"overflowing child reaches actual raster output");
    parent->setMasksToBounds(true); parent->skiaRender(canvas);
    expect(child->draws==1 && stats.rejected==1,"offscreen clipped subtree skips custom drawing");
    expect(canvas->getSaveCount()==saves,"rejection balances the canvas stack");
    parent->setFrame({8,8,32,32}); child->setFrame({4,4,12,12});
    parent->setAffineTransform(NXAffineTransform::rotationBy(25)); parent->skiaRender(canvas);
    expect(child->draws==2,"transformed visible clipped subtree still draws");
    parent->setBounds({4,6,32,32}); parent->skiaRender(canvas);
    expect(child->draws==3,"nonzero scroll bounds do not reject a visible subtree");
    // Scrolling and reparenting must reveal a previously rejected layer.
    auto root = new_shared<CALayer>(); root->setFrame({0,0,64,64}); root->addSublayer(parent);
    parent->setFrame({100,0,32,32}); root->skiaRender(canvas);
    const auto before = child->draws;
    root->setBounds({96,0,64,64}); root->skiaRender(canvas);
    expect(child->draws==before+1,"bounds-origin scrolling reveals previously rejected content");
    child->removeFromSuperlayer(); root->addSublayer(child); child->setFrame({100,8,12,12}); root->skiaRender(canvas);
    expect(child->draws==before+2,"reparenting leaves no persistent visibility cache");
    Internal::RenderPerformance::current = nullptr;
}
void ordering() {
    auto surface = SkSurfaces::Null(64,64);
    auto root = new_shared<CALayer>(); root->setFrame({0,0,64,64});
    std::vector<int> order;
    auto a=new_shared<Probe>(), b=new_shared<Probe>(), c=new_shared<Probe>();
    int id=0;
    for (const auto& child : {a,b,c}) { child->id=++id; child->order=&order; root->addSublayer(child); }
    Internal::RenderPerformance stats; Internal::RenderPerformance::current=&stats;
    root->skiaRender(surface->getCanvas());
    expect(order==std::vector<int>({1,2,3}) && stats.sorts==0,"equal-z insertion order needs no sorting");
    order.clear(); b->setZPosition(-1); root->skiaRender(surface->getCanvas());
    expect(order==std::vector<int>({2,1,3}) && stats.sorts==1,"changed z position sorts stably");
    order.clear(); b->setZPosition(0);
    a->callback=[root,b] { b->removeFromSuperlayer(); };
    root->skiaRender(surface->getCanvas());
    expect(order==std::vector<int>({1,2,3}),"drawing callbacks retain this traversal's membership snapshot");
    a->callback=nullptr; order.clear(); root->skiaRender(surface->getCanvas());
    expect(order==std::vector<int>({1,3}),"next traversal observes removal during drawing");
    Internal::RenderPerformance::current=nullptr;
}
class EdgeMask final:public Internal::LayerContentMask {
public:
    bool bounded=false;
    bool active(const CALayer&) const override{return true;}
    sk_sp<SkShader> contentShader(const CALayer&) const override {
        const SkPoint points[]={{0,0},{0,24}};
        const SkColor4f colors[]={{1,1,1,.4f},{1,1,1,1}};
        return SkShaders::LinearGradient(points,SkGradient(SkGradient::Colors(SkSpan(colors),{},SkTileMode::kClamp),{}));
    }
    std::optional<std::vector<SkRect>> affectedRegions(const CALayer&) const override {
        if(!bounded)return std::nullopt;
        // Overlap deliberately: the frozen prefix must be restored just once.
        return std::vector<SkRect>{SkRect::MakeXYWH(0,0,48,24),SkRect::MakeXYWH(32,0,32,24)};
    }
    void draw(SkCanvas* canvas,const CALayer& layer) const override {
        SkPaint paint;paint.setShader(contentShader(layer));
        canvas->drawRect(SkRect::MakeWH(64,64),paint);
    }
};
class BackdropProbe final:public CALayer {
public:
    int draws=0;
    void draw(SkCanvas* canvas) override {
        ++draws;
        const auto rect=SkRect::MakeWH(bounds().width(),bounds().height());
        SkPaint paint;paint.setImageFilter(SkImageFilters::Blur(2,2,SkTileMode::kClamp,nullptr));
        canvas->saveLayer(SkCanvas::SaveLayerRec(&rect,&paint,SkCanvas::kInitWithPrevious_SaveLayerFlag));
        canvas->restore();
    }
};
void boundedMasks() {
    for(const bool transparent:{false,true})for(const float rotation:{0.f,13.f})for(const float scale:{1.f,2.f,3.f})for(const bool fractional:{false,true})for(const bool fullscreen:{false,true}) {
        auto root=new_shared<CALayer>();root->setFrame({fullscreen?0.f:fractional?12.25f:12.f,fullscreen?0.f:fractional?14.5f:14.f,64,64});root->setMasksToBounds(true);
        root->setAffineTransform(NXAffineTransform::rotationBy(rotation));root->setBounds({3,7,64,64});
        auto child=new_shared<Probe>();child->setFrame({3,7,64,64});child->setBackgroundColor(UIColor(51,179,230,166));root->addSublayer(child);
        auto overlap=new_shared<CALayer>();overlap->setFrame({20,14,24,36});overlap->setBackgroundColor(UIColor(255,26,51,153));root->addSublayer(overlap);
        auto blur=new_shared<BackdropProbe>();blur->setFrame({12,10,36,20});root->addSublayer(blur);
        auto mask=std::make_shared<EdgeMask>();Internal::LayerContentMaskAccess::add(root,mask);
        SkBitmap reference;
        for(int variant=0;variant<3;++variant) {
            const int dimension=fullscreen?int(64*scale):300;
            auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(dimension,dimension));auto* canvas=surface->getCanvas();
            canvas->clear(transparent?0x40306090:0xff306090);canvas->scale(scale,scale);
            SkPaint background;background.setColor(0x8040ff30);canvas->drawRect(SkRect::MakeXYWH(0,0,70,30),background);
            mask->bounded=variant!=0;
            Internal::RenderPerformance stats;Internal::RenderPerformance::current=&stats;
            if(variant==2) {
                SkPictureRecorder recorder;root->skiaRender(recorder.beginRecording(SkRect::MakeWH(100,100)));
                canvas->drawPicture(recorder.finishRecordingAsPicture());
            }else root->skiaRender(canvas);
            Internal::RenderPerformance::current=nullptr;
            expect(stats.boundedMasks==uint64_t(variant==1&&fullscreen&&rotation==0),"only an equivalent hard-clipped device boundary uses bounded compositing");
            SkBitmap pixels;pixels.allocN32Pixels(dimension,dimension);surface->readPixels(pixels,0,0);
            if(!variant)reference=pixels;
            else {
                int maximum=0,mx=0,my=0;
                for(int y=0;y<dimension;++y)for(int x=0;x<dimension;++x) {
                    const auto a=*reference.getAddr32(x,y),b=*pixels.getAddr32(x,y);
                    for(int channel=0;channel<4;++channel)if(const int delta=std::abs(int((a>>(channel*8))&255)-int((b>>(channel*8))&255));delta>maximum){maximum=delta;mx=x;my=y;}
                }
                expect(maximum<=2,"bounded mask matches full isolation, including transparent backdrop and picture fallback");
                if(maximum>2)std::cerr<<"mask location "<<mx<<","<<my<<" colors "<<std::hex<<reference.getColor(mx,my)<<" "<<pixels.getColor(mx,my)<<std::dec<<" max error "<<maximum<<" transparent "<<transparent<<" rotation "<<rotation<<" scale "<<scale<<" variant "<<variant<<'\n';
            }
        }
        expect(child->draws==3&&blur->draws==3,"mask paths draw custom content and nested backdrop exactly once");
    }
}
}
int main() {
    clipping(); ordering(); boundedMasks();
    if (!failures) std::cout << "Render traversal tests passed\n";
    return failures?1:0;
}
