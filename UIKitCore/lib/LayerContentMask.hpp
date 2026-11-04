#pragma once
#include <CALayer.h>
#include "include/core/SkShader.h"
#include <algorithm>
namespace NXKit::Internal {
// Style-neutral viewport compositing, independent of the app's CALayer::mask.
class LayerContentMask {
public:
    virtual ~LayerContentMask()=default;
    virtual bool affectsSubtree() const { return false; }
    virtual bool active(const CALayer&) const=0;
    // Analytic masks avoid recording/rasterizing a full-viewport mask picture.
    // Custom drawing masks can retain the draw() fallback.
    virtual sk_sp<SkShader> contentShader(const CALayer&) const { return nullptr; }
    // Optional kit-owned compositing backdrop, in viewport coordinates. Core
    // supplies no color or material policy; ordinary masks leave it untouched.
    virtual void drawBackdrop(SkCanvas*,const CALayer&) const {}
    virtual void draw(SkCanvas*,const CALayer&) const=0;
};
class LayerContentMaskAccess {
public:
    static void add(const std::shared_ptr<CALayer>& layer,const std::shared_ptr<LayerContentMask>& mask) {
        std::erase_if(layer->_contentMasks,[](const auto& weak){return weak.expired();});
        layer->_contentMasks.push_back(mask);layer->setNeedsDisplay();
    }
};
}
