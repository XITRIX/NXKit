#include <CABlurLayer.h>
#include <tools/Tools.hpp>

#include "include/core/SkRRect.h"
#include "include/effects/SkImageFilters.h"

using namespace NXKit;

CABlurLayer::CABlurLayer(): CALayer() {
}

CABlurLayer::CABlurLayer(CABlurLayer* layer): CALayer(layer) {
    _blurRadius = layer->_blurRadius;
}

std::shared_ptr<CALayer> CABlurLayer::copy() {
    return new_shared<CABlurLayer>(this);
}

void CABlurLayer::draw(SkCanvas* context) {
    SkPaint blurPaint;
    blurPaint.setAntiAlias(true);

    blurPaint.setStyle(SkPaint::kFill_Style);
//    blurPaint.setStrokeWidth(10);
    
    sk_sp<SkImageFilter> newBlurFilter = SkImageFilters::Blur(_blurRadius, _blurRadius, SkTileMode::kClamp, nullptr);
    blurPaint.setImageFilter(std::move(newBlurFilter));
//
    context->save();

    SkRect rect = SkRect::MakeXYWH(0, 0, bounds().width(), bounds().height());
    float radii = cornerRadius();
    SkVector corners[] = {{radii, radii}, {radii, radii}, {radii, radii}, {radii, radii}};
    SkRRect rrect;
    rrect.setRectRadii(rect, corners);
    context->clipRRect(rrect, true);
//    // Make a separate layer using the blur filter, clipped to the middle rectangle's bounds
    SkCanvas::SaveLayerRec slr(&rect, &blurPaint, SkCanvas::kInitWithPrevious_SaveLayerFlag);
    context->saveLayer(slr);
    context->restore();
    
    context->drawColor(_backgroundTintColor.raw());
    context->restore();
}

void CABlurLayer::setBlurRadius(NXFloat blurRadius) {
    if (_blurRadius == blurRadius) return;
    onWillSet("blurRadius");
    _blurRadius = blurRadius;
}

std::optional<AnimatableProperty> CABlurLayer::value(std::string forKeyPath) {
    if (forKeyPath == "blurRadius") return _blurRadius;
    return CALayer::value(forKeyPath);
}

void CABlurLayer::update(std::shared_ptr<CALayer> presentation, std::shared_ptr<CABasicAnimation> animation, float progress) {
    if (!animation->keyPath.has_value() || !animation->fromValue.has_value()) return;

    auto keyPath = animation->keyPath.value();
    auto fromValue = animation->fromValue.value();

    if (keyPath == "blurRadius") {
        auto start = any_optional_cast<NXFloat>(fromValue);
        if (!start.has_value()) { return; }

        auto end = any_optional_cast<NXFloat>(animation->toValue);
        if (!end.has_value()) end = this->_blurRadius;

        std::static_pointer_cast<CABlurLayer>(presentation)->setBlurRadius(start.value() + (end.value() - start.value()) * progress);
    }

    CALayer::update(presentation, animation, progress);
}
