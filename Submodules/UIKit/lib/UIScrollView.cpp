//
// Created by Даниил Виноградов on 24.01.2025.
//
//
//  UIScrollView.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 12.03.2023.
//

#include <UIScrollView.h>
#include <CATransaction.h>
#include <UIScrollViewExtensions/SpringTimingParameters.h>
#include <UIScrollViewExtensions/RubberBand.h>

namespace NXKit {

UIScrollView::UIScrollView(NXRect frame): UIView(frame) {
    _panGestureRecognizer = new_shared<UIPanGestureRecognizer>();
    _panGestureRecognizer->onStateChanged = [this](auto){ onPanGestureStateChanged(); };
    addGestureRecognizer(_panGestureRecognizer);
    setClipsToBounds(true);

//    applyScrollIndicatorsStyle();
//    [horizontalScrollIndicator, verticalScrollIndicator].forEach {
//        $0.alpha = 0
//        addSubview($0)
//    }
}

//void UIScrollView::addSubview(std::shared_ptr<UIView> view) {
//    UIView::addSubview(view);
//    view->yoga->setPositionType(YGPositionTypeAbsolute);
//}

void UIScrollView::setContentOffset(NXPoint offset, bool animated) {
    if (offset == contentOffset()) return;

    // Cancel deceleration animations only when contentOffset gets set without animations.
    // Otherwise we might cancel any "bounds" animations which are not iniated from velocity scrolling.
    if (_isDecelerating && UIView::currentAnimationPrototype == nullptr) {
        cancelDecelerationAnimations();
    }

    setBounds(NXRect(offset, bounds().size));
    layoutScrollIndicatorsIfNeeded();

    // otherwise everything subscribing to scrollViewDidScroll is implicitly animated from velocity scroll
    CATransaction::begin();
    CATransaction::setDisableActions(!animated);
    if (!delegate.expired()) delegate.lock()->scrollViewDidScroll(shared_from_base<UIScrollView>());
    CATransaction::commit();
}

void UIScrollView::setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior contentInsetAdjustmentBehavior) {
    if (_contentInsetAdjustmentBehavior == contentInsetAdjustmentBehavior) return;
    _contentInsetAdjustmentBehavior = contentInsetAdjustmentBehavior;
    setNeedsLayout();
}

void UIScrollView::setBounceHorizontally(bool bounceHorizontally) {
    if (_bounceHorizontally == bounceHorizontally) return;
    _bounceHorizontally = bounceHorizontally;
    setNeedsLayout();
}

void UIScrollView::setBounceVertically(bool bounceVertically) {
    if (_bounceVertically == bounceVertically) return;
    _bounceVertically = bounceVertically;
    setNeedsLayout();
}

void UIScrollView::layoutMarginsDidChange() {
    auto delta = _lastLayoutMargins - layoutMargins();
    _lastLayoutMargins = layoutMargins();
    auto target = getBoundsCheckedContentOffset(contentOffset() + NXPoint(delta.left, delta.top));
    setContentOffset(target, false);
}

NXPoint UIScrollView::visibleContentOffset() {
    return (layer()->presentationOrSelf())->bounds().origin;
}

NXSize UIScrollView::contentSize() {
    if (subviews().empty()) return {};
    return subviews().front()->bounds().size;
}

NXPoint UIScrollView::getBoundsCheckedContentOffset(NXPoint newContentOffset) {
    auto contentNXSize = this->contentSize();
    auto contentHeight = contentNXSize.height;// fmaxf(contentNXSize.height, bounds().height());
    auto contentWidth = contentNXSize.width;// fmaxf(contentNXSize.width, bounds().width());

    auto allInsects = _contentInset;// + layoutMargins();

    bool contentWidthGreaterThenScrollBounds = contentWidth > bounds().width() -_contentInset.left - _contentInset.right;
    bool contentHeightGreaterThenScrollBounds = contentHeight > bounds().height() - _contentInset.top - _contentInset.bottom;

    switch (_contentInsetAdjustmentBehavior) {
        case UIScrollViewContentInsetAdjustmentBehavior::scrollableAxes: {
            if (contentWidthGreaterThenScrollBounds || _bounceHorizontally) {
                allInsects += UIEdgeInsets(0, layoutMargins().left, 0, layoutMargins().right);
            }
            if (contentHeightGreaterThenScrollBounds || _bounceVertically) {
                allInsects += UIEdgeInsets(layoutMargins().top, 0, layoutMargins().bottom, 0);
            }
            break;
        }
        case UIScrollViewContentInsetAdjustmentBehavior::always: {
            allInsects += layoutMargins();
            break;
        }
        case UIScrollViewContentInsetAdjustmentBehavior::never: {
            break;
        }
    }

    bool contentWidthGreaterThenScrollSafeArea = contentWidth > bounds().width() -allInsects.left - allInsects.right;
    bool contentHeightGreaterThenScrollSafeArea = contentHeight > bounds().height() - allInsects.top - allInsects.bottom;

    NXPoint target;
    if (!contentWidthGreaterThenScrollSafeArea) {
        target.x = - allInsects.left;
    } else {
        target.x = fminf(fmaxf(newContentOffset.x, -allInsects.left), (contentWidth + allInsects.right) - bounds().width());
    }

    if (!contentHeightGreaterThenScrollSafeArea) {
        target.y = - allInsects.top;
    } else {
        target.y = fminf(fmaxf(newContentOffset.y, -allInsects.top), (contentHeight + allInsects.bottom) - bounds().height());
    }

    return target;
}

NXRect UIScrollView::contentOffsetBounds() {
    auto contentNXSize = this->contentSize();
    auto contentHeight = contentNXSize.height;// fmaxf(contentNXSize.height, bounds().height());
    auto contentWidth = contentNXSize.width;// fmaxf(contentNXSize.width, bounds().width());

    auto allInsects = _contentInset;// + layoutMargins();

    bool contentWidthGreaterThenScrollBounds = contentWidth > bounds().width() -_contentInset.left - _contentInset.right;
    bool contentHeightGreaterThenScrollBounds = contentHeight > bounds().height() - _contentInset.top - _contentInset.bottom;

    switch (_contentInsetAdjustmentBehavior) {
        case UIScrollViewContentInsetAdjustmentBehavior::scrollableAxes: {
            if (contentWidthGreaterThenScrollBounds || _bounceHorizontally) {
                allInsects += UIEdgeInsets(0, layoutMargins().left, 0, layoutMargins().right);
            }
            if (contentHeightGreaterThenScrollBounds || _bounceVertically) {
                allInsects += UIEdgeInsets(layoutMargins().top, 0, layoutMargins().bottom, 0);
            }
            break;
        }
        case UIScrollViewContentInsetAdjustmentBehavior::always: {
            allInsects += layoutMargins();
            break;
        }
        case UIScrollViewContentInsetAdjustmentBehavior::never: {
            break;
        }
    }

    bool contentWidthGreaterThenScrollSafeArea = contentWidth > bounds().width() -allInsects.left - allInsects.right;
    bool contentHeightGreaterThenScrollSafeArea = contentHeight > bounds().height() - allInsects.top - allInsects.bottom;

    NXRect resBounds;
    if (!contentWidthGreaterThenScrollSafeArea) {
        resBounds.origin.x = - allInsects.left;
        resBounds.size.width = 0;
    } else {
        resBounds.origin.x = - allInsects.left;
        resBounds.size.width = (contentWidth + allInsects.left + allInsects.right) - bounds().width();
    }

    if (!contentHeightGreaterThenScrollSafeArea) {
        resBounds.origin.y = - allInsects.top;
        resBounds.size.height = 0;
    } else {
        resBounds.origin.y = - allInsects.top;
        resBounds.size.height = (contentHeight + allInsects.top + allInsects.bottom) - bounds().height();
    }

    return resBounds;
}

bool UIScrollView::shouldBounceHorizontally() {
    auto contentNXSize = this->contentSize();
    bool contentWidthGreaterThenScrollBounds = contentNXSize.width > bounds().width() -_contentInset.left - _contentInset.right;

    auto allInsects = _contentInset;// + layoutMargins();
    switch (_contentInsetAdjustmentBehavior) {
        case UIScrollViewContentInsetAdjustmentBehavior::scrollableAxes: {
            if (contentWidthGreaterThenScrollBounds || _bounceHorizontally) {
                allInsects += UIEdgeInsets(0, layoutMargins().left, 0, layoutMargins().right);
            }
            break;
        }
        case UIScrollViewContentInsetAdjustmentBehavior::always: {
            allInsects += layoutMargins();
            break;
        }
        case UIScrollViewContentInsetAdjustmentBehavior::never: {
            break;
        }
    }

    bool contentWidthGreaterThenScrollSafeArea = contentNXSize.width > bounds().width() -allInsects.left - allInsects.right;
    return contentWidthGreaterThenScrollSafeArea && _bounceHorizontally;
}

bool UIScrollView::shouldBounceVertically() {
    auto contentNXSize = this->contentSize();
    bool contentHeightGreaterThenScrollBounds = contentNXSize.height > bounds().height() - _contentInset.top - _contentInset.bottom;

    auto allInsects = _contentInset;// + layoutMargins();
    switch (_contentInsetAdjustmentBehavior) {
        case UIScrollViewContentInsetAdjustmentBehavior::scrollableAxes: {
            if (contentHeightGreaterThenScrollBounds || _bounceVertically) {
                allInsects += UIEdgeInsets(layoutMargins().top, 0, layoutMargins().bottom, 0);
            }
            break;
        }
        case UIScrollViewContentInsetAdjustmentBehavior::always: {
            allInsects += layoutMargins();
            break;
        }
        case UIScrollViewContentInsetAdjustmentBehavior::never: {
            break;
        }
    }
    bool contentHeightGreaterThenScrollSafeArea = contentNXSize.height > bounds().height() - allInsects.top - allInsects.bottom;
    return contentHeightGreaterThenScrollSafeArea && _bounceVertically;
}

void UIScrollView::onPan() {
    auto translation = _panGestureRecognizer->translationInView(shared_from_this());
//    _panGestureRecognizer->setTranslation(NXPoint(), shared_from_this());

    auto panGestureVelocity = _panGestureRecognizer->velocityIn(shared_from_this());
    weightedAverageVelocity = weightedAverageVelocity * 0.2 + panGestureVelocity * 0.8;

    auto offset = contentOffsetBounds();
    auto rubberBand = RubberBand(0.55f, frame().size, contentOffsetBounds());

    NXPoint clamped = getBoundsCheckedContentOffset(_initialContentOffset - translation);
    NXPoint target = rubberBand.clamp(_initialContentOffset - translation);

    if (!shouldBounceHorizontally()) {
        target.x = clamped.x;
    }
    if (!shouldBounceVertically()) {
        target.y = clamped.y;
    }

    setContentOffset(target, false);

//    auto newOffset = getBoundsCheckedContentOffset(_initialContentOffset - translation);
//    setContentOffset(newOffset, false);
}

void UIScrollView::onPanGestureStateChanged() {
    switch (_panGestureRecognizer->state()) {
        case UIGestureRecognizerState::possible: {
//            showScrollIndicators();
            cancelDeceleratingIfNeccessary();
            break;
        }
        case UIGestureRecognizerState::began: {
//            printf("Began\n");
            _initialContentOffset = contentOffset();
            break;
        }
        case UIGestureRecognizerState::changed: {
            if (!delegate.expired()) delegate.lock()->scrollViewWillBeginDragging(shared_from_base<UIScrollView>());
            onPan();
            break;
        }
        case UIGestureRecognizerState::ended: {
            startDeceleratingIfNecessary();
            weightedAverageVelocity = NXPoint();
            break;
        }
            // XXX: Spring back with animation:
            //case .ended, .cancelled:
            //if contentOffset.x < _contentInset.left {
            //    setContentOffset(CGNXPoint(x: _contentInset.left, y: contentOffset.y), animated: true)
            //}
        default: break;
    }
}

void UIScrollView::layoutScrollIndicatorsIfNeeded() {

}

void UIScrollView::showScrollIndicators() {

}

void UIScrollView::hideScrollIndicators() {

}

void UIScrollView::startDeceleratingIfNecessary() {
    // Only animate if instantaneous velocity is large enough
    // Otherwise we could animate after scrolling quickly, pausing for a few seconds, then letting go
    // TODO: Need to check velocity, it's too powerful
    auto velocity = NXPoint(-weightedAverageVelocity.x / 50, -weightedAverageVelocity.y / 50);
    if (!shouldBounceVertically()) {
        velocity.y = 0;
    }
    if (!shouldBounceHorizontally()) {
        velocity.x = 0;
    }

    auto decelerationRate = _decelerationRate.rawValue();
    auto threshold = 0.5f / traitCollection()->displayScale();// layer()->contentsScale();

    auto parameters = DecelerationTimingParameters(contentOffset(),velocity, decelerationRate, threshold);

    auto destination = parameters.destination();
    auto clippedDestination = getBoundsCheckedContentOffset(destination);
    bool isClipped = destination != clippedDestination;

    float duration;
    if (isClipped) {
        duration = parameters.durationTo(clippedDestination);
    } else {
        duration = parameters.duration();
    }

    _isDecelerating = true;
    _timerAnimation = std::make_shared<TimerAnimation>(duration, [this, parameters](float, double time) {
        setContentOffset(parameters.valueAt(time), false);
    }, [this, parameters, duration, isClipped](bool) {
        _isDecelerating = isClipped;

        if (isClipped) {
            auto velocity = parameters.velocityAt(duration);
            bounceWithVelocity(velocity);
        }
    });
//    auto nonBoundsCheckedScrollAnimationDistance = weightedAverageVelocity * dampingFactor; // hand-tuned
//    auto targetOffset = contentOffset() - nonBoundsCheckedScrollAnimationDistance;// getBoundsCheckedContentOffset(contentOffset() - nonBoundsCheckedScrollAnimationDistance);
//    auto distanceToBoundsCheckedTarget = contentOffset() - targetOffset;

//    auto velocityIsLargeEnoughToDecelerate = (velocity.magnitude() > 10);
//    auto willDecelerate = (velocityIsLargeEnoughToDecelerate && distanceToBoundsCheckedTarget.magnitude() > 0.0);
//
//    if (!delegate.expired()) delegate.lock()->scrollViewDidEndDragging(shared_from_base<UIScrollView>(), willDecelerate);
//    if (!willDecelerate) { hideScrollIndicators(); return; }
//
//    // https://ariya.io/2011/10/flick-list-with-its-momentum-scrolling-and-deceleration
//    // TODO: This value should be calculated from `self.decelerationRate` instead
//    // But actually we want to redo this function to avoid `UIView.animate` entirely,
//    // in which case we wouldn't need an animationTime at all.
//    auto animationTimeConstant = 0.325f * dampingFactor;
//
//    // This calculation is a weird approximation but it's close enough for now...
//    auto animationTime = logf(distanceToBoundsCheckedTarget.magnitude()) * animationTimeConstant;
//
//    UIViewAnimationOptions options = UIViewAnimationOptions(UIViewAnimationOptions::beginFromCurrentState | UIViewAnimationOptions::customEaseOut | UIViewAnimationOptions::allowUserInteraction);
//    UIView::animate(
//                    animationTime,
//                    0,
//                    options,
//                    [this, targetOffset]() {
//                        _isDecelerating = true;
//                        setContentOffset(targetOffset, false);
//                    },
//                    [this](bool) {
//                        _isDecelerating = false;
//                    }
//    );
}


void UIScrollView::bounceWithVelocity(NXPoint velocity) {
    auto restOffset = getBoundsCheckedContentOffset(contentOffset());
    auto displacement = contentOffset() - restOffset;
    auto threshold = 0.5f / traitCollection()->displayScale(); //layer()->contentsScale();
    auto spring = Spring(1, 100, 1);

    auto parameters = SpringTimingParameters(spring, displacement,velocity, threshold);

    auto duration = parameters.duration();
    _timerAnimation = std::make_shared<TimerAnimation>(duration, [this, restOffset, parameters](auto, float time){
        setContentOffset(restOffset + parameters.valueAt(time), false);
    }, [this](bool) {
        _isDecelerating = false;
    });
}

void UIScrollView::cancelDeceleratingIfNeccessary() {
    if (!_isDecelerating) { return; }

    // Get the presentation value from the current animation
    if (_timerAnimation) _timerAnimation->invalidate();
    setContentOffset(visibleContentOffset(), false);
    cancelDecelerationAnimations();
    _isDecelerating = false;
}

void UIScrollView::cancelDecelerationAnimations() {
//    if (!layer()->animations.isEmpty) {
//        layer.removeAnimation(forKey: "bounds")
//        horizontalScrollIndicator.layer.removeAnimation(forKey: "position")
//        verticalScrollIndicator.layer.removeAnimation(forKey: "position")
//    }
    layer()->removeAnimation("bounds");
}

//bool UIScrollView::applyXMLAttribute(std::string name, std::string value) {
//    if (UIView::applyXMLAttribute(name, value)) { return true; }
//
//    REGISTER_XIB_ATTRIBUTE(bounceVertically, valueToBool, setBounceVertically)
//    REGISTER_XIB_ATTRIBUTE(bounceHorizontally, valueToBool, setBounceHorizontally)
//
//    return false;
//}

void UIScrollView::layoutSubviews() {
    UIView::layoutSubviews();
//    setContentOffset(getBoundsCheckedContentOffset(contentOffset()), false);
}

}
