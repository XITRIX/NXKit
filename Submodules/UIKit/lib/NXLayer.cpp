//
// Created by Даниил Виноградов on 07.12.2024.
//

#include "NXLayer.h"
#include "include/core/SkRRect.h"

using namespace NXKit;

bool NXLayer::layerTreeIsDirty = true;

NXLayer::NXLayer() = default;

void NXLayer::setAnchorPoint(NXKit::NXPoint anchorPoint) {
    _anchorPoint = anchorPoint;
}

void NXLayer::setBackgroundColor(std::optional<NXColor> backgroundColor) {
    _backgroundColor = backgroundColor;
}

void NXLayer::setBounds(NXKit::NXRect bounds) {
    _bounds = bounds;
}

void NXLayer::setPosition(NXKit::NXPoint position) {
    _position = position;
}

void NXLayer::setCornerRadius(NXFloat cornerRadius) {
    _cornerRadius = cornerRadius;
}

void NXLayer::addSublayer(const std::shared_ptr<NXLayer>& layer) {
    layer->removeFromSuperlayer();
    _sublayers.push_back(layer);
    layer->superlayer = this->shared_from_this();
    NXLayer::layerTreeIsDirty = true;
}

void NXLayer::insertSublayerAt(const std::shared_ptr<NXLayer>& layer, int index) {
    layer->removeFromSuperlayer();
    _sublayers.insert(_sublayers.begin() + index, layer);
    layer->superlayer = this->shared_from_this();
    NXLayer::layerTreeIsDirty = true;
}

void NXLayer::insertSublayerAbove(const std::shared_ptr<NXLayer>& layer, const std::shared_ptr<NXLayer>& sibling) {
    // TODO: Need to implement
}

void NXLayer::insertSublayerBelow(const std::shared_ptr<NXLayer>& layer, const std::shared_ptr<NXLayer>& sibling) {
    auto itr = std::find(_sublayers.cbegin(), _sublayers.cend(), sibling);
    if (itr == _sublayers.cend()) { return; }

    layer->removeFromSuperlayer();
    _sublayers.insert(itr, layer);
    layer->superlayer = this->shared_from_this();
    NXLayer::layerTreeIsDirty = true;
}

void NXLayer::removeFromSuperlayer() {
    auto super = superlayer.lock();
    if (super == nullptr) return;

//    // If it's mask - remove
//    if (super->mask.get() == this) {
//        super->mask = nullptr;
//        return;
//    }

    // Find and remove this from superlayer
    super->_sublayers.erase(std::remove(super->_sublayers.begin(), super->_sublayers.end(), shared_from_this()), super->_sublayers.end());
    NXLayer::layerTreeIsDirty = true;
}

void NXLayer::skiaRender(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(_backgroundColor->color);
    paint.setAntiAlias(true);

    SkRect rect = SkRect::MakeXYWH(_position.x, _position.y, _bounds.width(), _bounds.height());
    SkRRect rrect;
    float radii = _cornerRadius;
    SkVector corners[] = {{radii, radii}, {radii, radii}, {radii, radii}, {radii, radii}};
    rrect.setRectRadii(rect, corners);
    canvas->drawRRect(rrect, paint);

    for (const auto& sublayer: _sublayers) {
        sublayer->skiaRender(canvas);
    }
}