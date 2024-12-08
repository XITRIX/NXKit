#pragma once

#include "include/core/SkCanvas.h"

#include <Geometry.h>
#include <NXColor.h>

#include <tools/SharedBase.hpp>
#include <optional>
#include <vector>

namespace NXKit {

class NXLayer: public enable_shared_from_this<NXLayer> {
public:
    NXLayer();
    ~NXLayer() {}

    // Getter Setters
    void setAnchorPoint(NXPoint anchorPoint);
    [[nodiscard]] NXPoint anchorPoint() const { return _anchorPoint; }

    void setBounds(NXRect bounds);
    [[nodiscard]] NXRect bounds() const { return _bounds; }

    void setPosition(NXPoint position);
    [[nodiscard]] NXPoint position() const { return _position; }

    void setBackgroundColor(std::optional<NXColor> backgroundColor);
    [[nodiscard]] std::optional<NXColor> backgroundColor() const { return _backgroundColor; }

    void setCornerRadius(NXFloat cornerRadius);
    [[nodiscard]] NXFloat cornerRadius() const { return _cornerRadius; }


    // Layers
    [[nodiscard]] std::vector<std::shared_ptr<NXLayer>> sublayers() { return _sublayers; }

    void addSublayer(const std::shared_ptr<NXLayer>& layer);
    void insertSublayerAt(const std::shared_ptr<NXLayer>& layer, int index);
    void insertSublayerAbove(const std::shared_ptr<NXLayer>& layer, const std::shared_ptr<NXLayer>& sibling);
    void insertSublayerBelow(const std::shared_ptr<NXLayer>& layer, const std::shared_ptr<NXLayer>& sibling);

    void removeFromSuperlayer();

    void skiaRender(SkCanvas* canvas);
private:

    std::weak_ptr<NXLayer> superlayer;
    std::vector<std::shared_ptr<NXLayer>> _sublayers;

    // Animatable
    NXPoint _anchorPoint = NXPoint(0.5f, 0.5f);
    NXPoint _position;
    NXRect _bounds;
    NXFloat _cornerRadius = 0;
    std::optional<NXColor> _backgroundColor;

    /**
     Indicates whether a layer somewhere has changed since the last render pass.

     The current implementation of this is quite simple and doesn't check whether the layer is actually in
     the layer hierarchy or not. In theory this means that we're wasting render passes if users frequently
     update layers that aren't in the tree. In practice it's not expected that UIKit users would do that
     often enough for us to care about it.
    **/
    static bool layerTreeIsDirty;
};

}
