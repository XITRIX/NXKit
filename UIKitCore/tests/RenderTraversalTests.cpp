#include <CALayer.h>
#include "RenderPerformance.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
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
}
int main() {
    clipping(); ordering();
    if (!failures) std::cout << "Render traversal tests passed\n";
    return failures?1:0;
}
