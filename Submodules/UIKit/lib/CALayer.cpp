//
// Created by Даниил Виноградов on 07.12.2024.
//

#include "CALayer.h"
#include "include/core/SkRRect.h"

using namespace NXKit;

bool CALayer::layerTreeIsDirty = true;

CALayer::CALayer() = default;

void CALayer::setAnchorPoint(NXKit::NXPoint anchorPoint) {
    if (_anchorPoint == anchorPoint) return;
    onWillSet("anchorPoint");
    _anchorPoint = anchorPoint;
}

void CALayer::setBackgroundColor(std::optional<UIColor> backgroundColor) {
    if (_backgroundColor == backgroundColor) return;
    onWillSet("backgroundColor");
    _backgroundColor = backgroundColor;
}

void CALayer::setOpacity(NXFloat opacity) {
    if (_opacity == opacity) return;
    onWillSet("opacity");
    _opacity = opacity;
}

void CALayer::setBounds(NXKit::NXRect bounds) {
    if (_bounds == bounds) return;
    onWillSet("bounds");
    _bounds = bounds;
}

void CALayer::setPosition(NXKit::NXPoint position) {
    if (_position == position) return;
    onWillSet("position");
    _position = position;
}

void CALayer::setZPosition(NXKit::NXFloat zPosition) {
    if (_zPosition == zPosition) return;
    onWillSet("zPosition");
    _zPosition = zPosition;
}

void CALayer::setCornerRadius(NXFloat cornerRadius) {
    if (_cornerRadius == cornerRadius) return;
    onWillSet("cornerRadius");
    _cornerRadius = cornerRadius;
}

void CALayer::setTransform(NXTransform3D transform) {
    if (_transform == transform) return;
    onWillSet("transform");
    _transform = transform;
}

void CALayer::setContents(std::shared_ptr<CGImage> contents) {
    _contents = contents;
    CALayer::layerTreeIsDirty = true;
}

void CALayer::setMask(std::shared_ptr<CALayer> mask) {
    if (this->_mask) {
        this->_mask->_superlayer.reset();
    }

    this->_mask = mask;
    if (mask) _mask->_superlayer = shared_from_this();
}

void CALayer::addSublayer(const std::shared_ptr<CALayer>& layer) {
    layer->removeFromSuperlayer();
    _sublayers.push_back(layer);
    layer->_superlayer = this->shared_from_this();
    CALayer::layerTreeIsDirty = true;
}

void CALayer::insertSublayerAt(const std::shared_ptr<CALayer>& layer, int index) {
    layer->removeFromSuperlayer();
    _sublayers.insert(_sublayers.begin() + index, layer);
    layer->_superlayer = this->shared_from_this();
    CALayer::layerTreeIsDirty = true;
}

void CALayer::insertSublayerAbove(const std::shared_ptr<CALayer>& layer, const std::shared_ptr<CALayer>& sibling) {
    // TODO: Need to implement
}

void CALayer::insertSublayerBelow(const std::shared_ptr<CALayer>& layer, const std::shared_ptr<CALayer>& sibling) {
    auto itr = std::find(_sublayers.cbegin(), _sublayers.cend(), sibling);
    if (itr == _sublayers.cend()) { return; }

    layer->removeFromSuperlayer();
    _sublayers.insert(itr, layer);
    layer->_superlayer = this->shared_from_this();
    CALayer::layerTreeIsDirty = true;
}

void CALayer::removeFromSuperlayer() {
    auto super = _superlayer.lock();
    if (super == nullptr) return;

    // If it's mask - remove
    if (super->_mask.get() == this) {
        super->_mask = nullptr;
        return;
    }

    // Find and remove this from superlayer
    super->_sublayers.erase(std::remove(super->_sublayers.begin(), super->_sublayers.end(), shared_from_this()), super->_sublayers.end());
    CALayer::layerTreeIsDirty = true;
}

void CALayer::skiaRender(SkCanvas* canvas) {
    canvas->save();

    auto matrix = CATransform3DMakeTranslation(_position.x, _position.y, _zPosition)
        .concat(_transform);

    // Set Origin matrix
    canvas->concat(matrix.toSkM44());
    canvas->save();

    // Set Anchor matrix
    canvas->concat(CATransform3DMakeTranslation(-_bounds.width() * _anchorPoint.x, -_bounds.height() * _anchorPoint.y, 0).toSkM44());

    SkPaint paint;
    paint.setAntiAlias(true);
    SkRect rect = SkRect::MakeXYWH(0, 0, _bounds.width(), _bounds.height());

    // Background color
    if (_backgroundColor.has_value()) {
        paint.setColor(_backgroundColor->color);

        SkRRect rrect;
        float radii = _cornerRadius;
        SkVector corners[] = {{radii, radii}, {radii, radii}, {radii, radii}, {radii, radii}};
        rrect.setRectRadii(rect, corners);
        canvas->drawRRect(rrect, paint);
    }

    // Content
    if (_contents) {
        auto contentsGravity = ContentsGravityTransformation(this);
        canvas->drawImageRect(_contents.get()->pointee, rect, SkSamplingOptions());
//        GPU_SetAnchor(_contents->pointee, _anchorPoint.x, _anchorPoint.y);
//        GPU_SetRGBA(_contents->pointee, 255, 255, 255, _opacity * 255);
//
//        GPU_BlitTransform(
//            _contents->pointee,
//            NULL,
//            renderer,
//            contentsGravity.offset.x,
//            contentsGravity.offset.y,
//            0, // rotation in degrees
//            contentsGravity.scale.width / contentsScale,
//            contentsGravity.scale.height / contentsScale
//        );
    }

    // Reset Anchor to Origin matrix
    canvas->restore();
    for (const auto& sublayer: _sublayers) {
        sublayer->skiaRender(canvas);
    }

    canvas->restore();
}

NXRect CALayer::getFrame() {
    // Create a rectangle based on `bounds.size` * `transform` at `position` offset by `anchorPoint`
    auto transformedBounds = _bounds.applying(_transform);

    auto anchorPointOffset = NXPoint(
            transformedBounds.width() * _anchorPoint.x,
            transformedBounds.height() * _anchorPoint.y
    );

    return NXRect(_position.x - anchorPointOffset.x,
                _position.y - anchorPointOffset.y,
                transformedBounds.width(),
                transformedBounds.height());
}

void CALayer::onWillSet(std::string keyPath) {
    CALayer::layerTreeIsDirty = true;
    auto animationKey = keyPath;

//    auto animation = std::static_pointer_cast<CABasicAnimation>(actionForKey(animationKey));
//    if (animation &&
//        (this->hasBeenRenderedInThisPartOfOverallLayerHierarchy
//            || animation->wasCreatedInUIAnimateBlock()) &&
//        !this->isPresentationForAnotherLayer &&
//        !CATransaction::disableActions())
//    {
//        add(animation, animationKey);
//    }
}