//
// Created by Даниил Виноградов on 07.12.2024.
//

#include "CALayer.h"
#include <UIView.h>
#include <CATransaction.h>
#include <tools/Tools.hpp>
#include "include/core/SkRRect.h"

#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"

using namespace NXKit;

bool CALayer::layerTreeIsDirty = true;
NXFloat CALayer::defaultAnimationDuration = 0.3f;

CALayer::CALayer() = default;

CALayer::CALayer(CALayer* layer) {
    delegate = layer->delegate;
    _bounds = layer->_bounds;
    _transform = layer->_transform;
    _position = layer->_position;
    _anchorPoint = layer->_anchorPoint;
    _opacity = layer->_opacity;
    _backgroundColor = layer->_backgroundColor;
    _isHidden = layer->_isHidden;
    _cornerRadius = layer->_cornerRadius;
//    borderWidth = layer->borderWidth;
//    borderColor = layer->borderColor;
//    shadowColor = layer->shadowColor;
//    shadowPath = layer->shadowPath;
//    shadowOffset = layer->shadowOffset;
//    shadowRadius = layer->shadowRadius;
//    shadowOpacity = layer->shadowOpacity;
    _mask = layer->_mask;
    _masksToBounds = layer->_masksToBounds;
    _contents = layer->_contents; // XXX: we should make a copy here
    _contentsScale = layer->_contentsScale;
    _superlayer = layer->_superlayer;
    _sublayers = layer->_sublayers;
    _contentsGravity = layer->_contentsGravity;
}

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

void CALayer::setAffineTransform(NXAffineTransform transform) {
    this->setTransform(NXTransform3DMakeAffineTransform(transform));
}

NXAffineTransform CALayer::affineTransform() {
    return NXTransform3DGetAffineTransform(_transform);
}

void CALayer::setHidden(bool hidden) {
    _isHidden = hidden;
    CALayer::layerTreeIsDirty = true;
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

void CALayer::draw(SkCanvas* context) {}

void CALayer::skiaRender(SkCanvas* canvas) {
    // Do not render is hidden
    if (_isHidden || _opacity < 0.001f) return;

    // Initial save 1
    canvas->save();

    auto matrix = CATransform3DMakeTranslation(_position.x, _position.y, _zPosition)
        .concat(_transform);

    // Set Origin matrix
    canvas->concat(matrix.toSkM44());

    // Masks To Bounds
    if (_masksToBounds) {
        SkRect rect = SkRect::MakeXYWH(0, 0, _bounds.width(), _bounds.height());
        float radii = _cornerRadius;
        SkVector corners[] = {{radii, radii}, {radii, radii}, {radii, radii}, {radii, radii}};
        SkRRect rrect;
        rrect.setRectRadii(rect, corners);
        canvas->clipRRect(rrect, true);
    }

    // Origin matrix save 2
    canvas->save();

    // Set Anchor matrix
    canvas->concat(CATransform3DMakeTranslation(-_bounds.width() * _anchorPoint.x, -_bounds.height() * _anchorPoint.y, 0).toSkM44());

    SkPaint paint;
    paint.setAntiAlias(true);

    // Opacity save 3
    if (_opacity < 1) {
        canvas->saveLayerAlphaf(nullptr, _opacity);
    }

    // Background color
    if (_backgroundColor.has_value()) {
        paint.setColor(_backgroundColor->color);

        SkRect rect = SkRect::MakeXYWH(0, 0, _bounds.width(), _bounds.height());
        SkRRect rrect;
        float radii = _cornerRadius;
        SkVector corners[] = {{radii, radii}, {radii, radii}, {radii, radii}, {radii, radii}};
        rrect.setRectRadii(rect, corners);
        canvas->drawRRect(rrect, paint);
    }

    // Contents
    if (_contents) {
        auto contentsGravity = ContentsGravityTransformation(this);
        auto width = _contents->size().width * contentsGravity.scale.width / _contentsScale;
        auto height = _contents->size().height * contentsGravity.scale.height / _contentsScale;
        auto x = (_bounds.size.width - width) / 2.0 + contentsGravity.offset.x;
        auto y = (_bounds.size.height - height) / 2.0 + contentsGravity.offset.y;

        // Contents maxrix save 4
        canvas->save();
        canvas->translate(x, y);

        canvas->drawImageRect(_contents.get()->pointee, {
            float(0),
            float(0),
            float(width),
            float(height)
        }, SkSamplingOptions(), nullptr);

        // Contents maxrix save 4 // restore
        canvas->restore();
    }

    // Opacity save 3 // restore
    if (_opacity < 1) {
        canvas->restore();
    }

    draw(canvas);

    // Reset Anchor to Origin matrix
    // Origin matrix save 2 // restore
    canvas->restore();
    for (const auto& sublayer: _sublayers) {
        sublayer->presentationOrSelf()->skiaRender(canvas);
    }

    // Initial save 1 // restore
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

void CALayer::setFrame(NXRect frame) {
    setPosition(NXPoint(frame.origin.x + (frame.width() * _anchorPoint.x),
                     frame.origin.y + (frame.height() * _anchorPoint.y)));

    auto inverseTransformOpt = affineTransform().inverted();
    if (!inverseTransformOpt.has_value()) {
//        assertionFailure("You tried to set the frame of a CALayer whose transform cannot be inverted. This is undefined behaviour.");
        return;
    }
    auto inverseTransform = inverseTransformOpt.value();


    // If we are shrinking the view with a transform and then setting a
    // new frame, the layer's actual `bounds` is bigger (and vice-versa):
    auto nonTransformedBoundSize = frame.applying(inverseTransform).size;

    auto bounds = _bounds;
    bounds.size = nonTransformedBoundSize;
    setBounds(bounds);
}

std::shared_ptr<CALayer> CALayer::copy() {
    return new_shared<CALayer>(this);
}

std::shared_ptr<CAAction> CALayer::actionForKey(std::string event) {
    if (!delegate.expired()) return delegate.lock()->actionForKey(event);
    return CALayer::defaultActionForKey(event);
}

std::shared_ptr<CABasicAnimation> CALayer::defaultActionForKey(std::string event) {
    auto animation = new_shared<CABasicAnimation>(event);
    animation->duration = CATransaction::animationDuration();
    return animation;
}

std::shared_ptr<CALayer> CALayer::createPresentation() {
    auto _copy = copy();
    _copy->isPresentationForAnotherLayer = true;
    return _copy;
}

void CALayer::display() {
    if (!delegate.expired()) delegate.lock()->display(shared_from_this());
}

// MARK: - Animations
void CALayer::add(std::shared_ptr<CABasicAnimation> animation, std::string keyPath) {
    auto copy = new_shared<CABasicAnimation>(animation.get());
    copy->creationTime = Timer();

    // animation.fromValue is optional, set it to currently visible state if nil
    if (!copy->fromValue.has_value() && copy->keyPath.has_value()) {
        auto presentation = _presentation;
        if (!presentation) presentation = shared_from_this();

        copy->fromValue = presentation->value(keyPath);
    }

    if (copy->animationGroup)
        copy->animationGroup->queuedAnimations += 1;

    if (animations.count(keyPath) && animations[keyPath]->animationGroup)
        animations[keyPath]->animationGroup->animationDidStop(false);

    auto isEmpty = animations.empty();
    animations[keyPath] = copy;
    onDidSetAnimations(isEmpty);
}

void CALayer::removeAllAnimations() {
    auto isEmpty = animations.empty();
    animations.clear();
    onDidSetAnimations(isEmpty);
}

void CALayer::removeAnimation(std::string forKey) {
    auto isEmpty = animations.empty();
    animations.erase(forKey);
    onDidSetAnimations(isEmpty);
}

void CALayer::onWillSet(std::string keyPath) {
    CALayer::layerTreeIsDirty = true;
    auto animationKey = keyPath;

    auto animation = std::static_pointer_cast<CABasicAnimation>(actionForKey(animationKey));
    if (animation &&
        (this->hasBeenRenderedInThisPartOfOverallLayerHierarchy
            || animation->wasCreatedInUIAnimateBlock()) &&
        !this->isPresentationForAnotherLayer &&
        !CATransaction::disableActions())
    {
        add(animation, animationKey);
    }
}

void CALayer::onDidSetAnimations(bool wasEmpty) {
    if (!animations.empty() && wasEmpty) {
        UIView::layersWithAnimations.insert(shared_from_this());
        _presentation = createPresentation();
    } else if (animations.empty() && !wasEmpty) {
        _presentation = nullptr;
        UIView::layersWithAnimations.erase(shared_from_this());
    }
}

std::optional<AnimatableProperty> CALayer::value(std::string forKeyPath) {
    if (forKeyPath == "backgroundColor") return _backgroundColor;
    if (forKeyPath == "opacity") return _opacity;
    if (forKeyPath == "bounds") return _bounds;
    if (forKeyPath == "transform") return _transform;
    if (forKeyPath == "position") return _position;
    if (forKeyPath == "anchorPoint") return _anchorPoint;
    if (forKeyPath == "cornerRadius") return _cornerRadius;
    return std::nullopt;
}

std::shared_ptr<CALayer> CALayer::presentationOrSelf() {
    if (_presentation) return _presentation;
    return shared_from_this();
}

void CALayer::animateAt(Timer currentTime) {
    auto presentation = createPresentation();

    auto animationsCopy = animations;
    for (auto& animation: animationsCopy) {
        auto animationProgress = animation.second->progressFor(currentTime);
        update(presentation, animation.second, animationProgress);

        if (animationProgress == 1 && animation.second->isRemovedOnCompletion) {
            removeAnimation(animation.first);
            if (animation.second->animationGroup)
                animation.second->animationGroup->animationDidStop(true);
        }
    }

    this->_presentation = animations.empty() ? nullptr : presentation;
}

// Writing into `presentation->_...` cause we don't need onWillSet to be triggered
void CALayer::update(std::shared_ptr<CALayer> presentation, std::shared_ptr<CABasicAnimation> animation, float progress) {
    if (!animation->keyPath.has_value() || !animation->fromValue.has_value()) return;

    auto keyPath = animation->keyPath.value();
    auto fromValue = animation->fromValue.value();

    if (keyPath == "backgroundColor") {
        auto start = any_optional_cast<std::optional<UIColor>>(fromValue);
        if (!start.has_value()) { return; }

        auto end = any_optional_cast<std::optional<UIColor>>(animation->toValue);
        if (!end.has_value()) end = this->_backgroundColor;
        if (!end.has_value()) end = UIColor::clear;

        presentation->setBackgroundColor(start.value()->interpolationTo(end.value().value(), progress));
    }
    if (keyPath == "position") {
        auto start = any_optional_cast<NXPoint>(fromValue);
        if (!start.has_value()) { return; }

        auto end = any_optional_cast<NXPoint>(animation->toValue);
        if (!end.has_value()) end = this->_position;

        presentation->setPosition(start.value() + (end.value() - start.value()) * progress);
    }
    if (keyPath == "anchorPoint") {
        auto start = any_optional_cast<NXPoint>(fromValue);
        if (!start.has_value()) { return; }

        auto end = any_optional_cast<NXPoint>(animation->toValue);
        if (!end.has_value()) end = this->_anchorPoint;

        presentation->setAnchorPoint(start.value() + (end.value() - start.value()) * progress);
    }
    if (keyPath == "bounds") {
        auto start = any_optional_cast<NXRect>(fromValue);
        if (!start.has_value()) { return; }

        auto end = any_optional_cast<NXRect>(animation->toValue);
        if (!end.has_value()) end = this->_bounds;

        presentation->setBounds(start.value() + (end.value() - start.value()) * progress);
    }
    if (keyPath == "opacity") {
        auto start = any_optional_cast<float>(fromValue);
        if (!start.has_value()) { return; }

        auto end = any_optional_cast<float>(animation->toValue);
        if (!end.has_value()) end = this->_opacity;

        presentation->setOpacity(start.value() + (end.value() - start.value()) * progress);
    }
    if (keyPath == "cornerRadius") {
        auto start = any_optional_cast<float>(fromValue);
        if (!start.has_value()) { return; }

        auto end = any_optional_cast<float>(animation->toValue);
        if (!end.has_value()) end = this->_cornerRadius;

        presentation->setCornerRadius(start.value() + (end.value() - start.value()) * progress);
    }
    if (keyPath == "transform") {
        auto start = any_optional_cast<NXTransform3D>(fromValue);
        if (!start.has_value()) { return; }

        auto end = any_optional_cast<NXTransform3D>(animation->toValue);
        if (!end.has_value()) end = this->_transform;

//        presentation->_transform = start.value() + (end.value() - start.value()) * progress;
//        presentation->_transform = start.value() + (end.value() - start.value()).interpolate(progress);
        presentation->setTransform(start.value().interpolateTo(end.value(), progress));
    }
}
