#pragma once

#include "include/core/SkCanvas.h"

#include <Geometry.h>
#include <NXTransform3D.h>
#include <NXAffineTransform.h>
#include <ContentsGravityTransformation.h>
#include <CGImage.h>
#include <UIColor.h>

#include <tools/SharedBase.hpp>
#include <optional>
#include <vector>

namespace NXKit {

class CALayer: public enable_shared_from_this<CALayer> {
public:
    CALayer();
    ~CALayer() {}

    // Getter Setters
    void setContentsGravity(CALayerContentsGravity contentsGravity) { _contentsGravity = contentsGravity; }
    [[nodiscard]] CALayerContentsGravity contentsGravity() const { return _contentsGravity; }

    void setContentsScale(NXFloat contentsScale) { _contentsScale = contentsScale; }
    [[nodiscard]] NXFloat contentsScale() const { return _contentsScale; }

    void setAnchorPoint(NXPoint anchorPoint);
    [[nodiscard]] NXPoint anchorPoint() const { return _anchorPoint; }

    void setOpacity(NXFloat opacity);
    [[nodiscard]] NXFloat opacity() const { return _opacity; }

    void setBounds(NXRect bounds);
    [[nodiscard]] NXRect bounds() const { return _bounds; }

    void setPosition(NXPoint position);
    [[nodiscard]] NXPoint position() const { return _position; }

    void setZPosition(NXFloat position);
    [[nodiscard]] NXFloat zPosition() { return _zPosition; }

    void setBackgroundColor(std::optional<UIColor> backgroundColor);
    [[nodiscard]] std::optional<UIColor> backgroundColor() const { return _backgroundColor; }

    void setCornerRadius(NXFloat cornerRadius);
    [[nodiscard]] NXFloat cornerRadius() const { return _cornerRadius; }

    void setTransform(NXTransform3D transform);
    [[nodiscard]] NXTransform3D transform() const { return _transform; }

    void setMask(std::shared_ptr<CALayer> mask);
    [[nodiscard]] std::shared_ptr<CALayer> mask() const { return _mask; }

    void setContents(std::shared_ptr<CGImage> contents);
    [[nodiscard]] std::shared_ptr<CGImage> contents() { return _contents; }

    NXRect getFrame();
    void setFrame(NXRect frame);

    // Layers
    [[nodiscard]] std::vector<std::shared_ptr<CALayer>> sublayers() { return _sublayers; }

    void addSublayer(const std::shared_ptr<CALayer>& layer);
    void insertSublayerAt(const std::shared_ptr<CALayer>& layer, int index);
    void insertSublayerAbove(const std::shared_ptr<CALayer>& layer, const std::shared_ptr<CALayer>& sibling);
    void insertSublayerBelow(const std::shared_ptr<CALayer>& layer, const std::shared_ptr<CALayer>& sibling);

    void removeFromSuperlayer();

    void onWillSet(std::string keyPath);

    void skiaRender(SkCanvas* canvas);
private:

    /// Defaults to 1.0 but if the layer is associated with a view,
    /// the view sets this value to match the screen.
    NXFloat _contentsScale = 1.0f;
    CALayerContentsGravity _contentsGravity = CALayerContentsGravity::resize;

    std::weak_ptr<CALayer> _superlayer;
    std::vector<std::shared_ptr<CALayer>> _sublayers;
    std::shared_ptr<CALayer> _mask;

    std::shared_ptr<CGImage> _contents;

    // Animatable
    NXPoint _anchorPoint = NXPoint(0.5, 0.5);
    NXPoint _position;
    NXFloat _opacity = 1;
    NXFloat _zPosition = 0.0;
    NXRect _bounds;
    NXFloat _cornerRadius = 0;
    std::optional<UIColor> _backgroundColor;
    NXTransform3D _transform = NXTransform3D::identity;

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