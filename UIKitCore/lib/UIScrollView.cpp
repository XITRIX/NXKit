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
#include <UIWindow.h>
#include <tools/IBTools.h>
#include <UIScrollViewExtensions/SpringTimingParameters.h>
#include <UIScrollViewExtensions/RubberBand.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <unordered_set>

namespace NXKit {

namespace {

constexpr NXFloat kFocusVisibilityTolerance = 0.5f;
constexpr NXFloat kNaturalFocusScrollSpeed = 1000.0f;

bool isDirectionalFocusHeading(UIFocusHeading heading) {
    return heading == UIFocusHeading::up
        || heading == UIFocusHeading::down
        || heading == UIFocusHeading::left
        || heading == UIFocusHeading::right;
}

} // namespace

UIScrollView::UIScrollView(const NXRect frame): UIView(frame) {
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

UIScrollView::~UIScrollView() {
    if (_naturalFocusDisplayLink) {
        _naturalFocusDisplayLink->invalidate();
    }
}

bool UIScrollView::canBecomeFocused() {
    return _isScrollEnabled
        && !isHidden()
        && alpha() > 0
        && isUserInteractionEnabled();
}

std::shared_ptr<UIFocusItem> UIScrollView::searchForFocus() {
    const auto descendant = findFocusableDescendant(
        _focusTrackingMode == UIScrollViewFocusTrackingMode::natural
    );
    if (descendant) {
        return descendant;
    }
    return canBecomeFocused()
        ? shared_from_base<UIFocusItem>()
        : nullptr;
}

std::shared_ptr<UIView> UIScrollView::findFocusableDescendant(
    bool visibleOnly
) {
    const auto scrollView = shared_from_base<UIScrollView>();
    std::unordered_set<const UIFocusEnvironment*> visited;
    std::function<std::shared_ptr<UIView>(
        const std::shared_ptr<UIFocusEnvironment>&
    )> visit;
    visit = [this, &scrollView, visibleOnly, &visited, &visit](
        const std::shared_ptr<UIFocusEnvironment>& environment
    ) -> std::shared_ptr<UIView> {
        if (!environment || !visited.insert(environment.get()).second) {
            return nullptr;
        }

        const auto view = std::dynamic_pointer_cast<UIView>(environment);
        if (view && view.get() != this) {
            if (!view->isDescendantOf(scrollView)) {
                return nullptr;
            }
            if (view->canBecomeFocused()
                && (!visibleOnly || isFullyVisibleForFocus(view))) {
                return view;
            }
        }

        for (const auto& preferred : environment->preferredFocusEnvironments()) {
            if (const auto result = visit(preferred)) {
                return result;
            }
        }
        if (!view) {
            return nullptr;
        }
        for (const auto& child : view->subviews()) {
            if (const auto result = visit(child)) {
                return result;
            }
        }
        return nullptr;
    };

    for (const auto& preferred : preferredFocusEnvironments()) {
        if (const auto result = visit(preferred)) {
            return result;
        }
    }
    for (const auto& child : subviews()) {
        if (const auto result = visit(child)) {
            return result;
        }
    }
    return nullptr;
}

std::shared_ptr<UIView> UIScrollView::nextFocusableDescendant(
    const std::shared_ptr<UIView>& current,
    UIFocusHeading heading
) {
    if (!current || !containsFocusView(current) || current.get() == this) {
        return nullptr;
    }

    auto cursor = current;
    const auto scrollView = shared_from_base<UIScrollView>();
    while (cursor && cursor != scrollView) {
        const auto parent = cursor->superview().lock();
        if (!parent) {
            break;
        }
        const auto candidate = parent->getNextFocusItem(cursor, heading);
        if (candidate) {
            const auto candidateView =
                std::dynamic_pointer_cast<UIView>(candidate);
            return candidateView && candidateView.get() != this
                    && candidateView->isDescendantOf(scrollView)
                ? candidateView
                : nullptr;
        }
        cursor = parent;
    }
    return nullptr;
}

NXRect UIScrollView::focusRectInContent(
    const std::shared_ptr<UIView>& view
) {
    if (!view || view.get() == this || !containsFocusView(view)) {
        return NXRect();
    }
    return {
        view->convertToView(
            view->bounds().origin,
            shared_from_base<UIScrollView>()
        ),
        view->bounds().size
    };
}

NXRect UIScrollView::focusViewport(NXPoint offset) {
    auto insets = effectiveContentInsets();
    const auto safeArea = safeAreaInsets();
    insets.top = std::max(insets.top, safeArea.top);
    insets.left = std::max(insets.left, safeArea.left);
    insets.bottom = std::max(insets.bottom, safeArea.bottom);
    insets.right = std::max(insets.right, safeArea.right);
    return {
        offset.x + insets.left,
        offset.y + insets.top,
        std::max<NXFloat>(
            0,
            bounds().width() - insets.left - insets.right
        ),
        std::max<NXFloat>(
            0,
            bounds().height() - insets.top - insets.bottom
        )
    };
}

bool UIScrollView::isFullyVisibleForFocus(
    const std::shared_ptr<UIView>& view
) {
    const auto item = focusRectInContent(view);
    const auto viewport = focusViewport(contentOffset());
    return item.width() <= viewport.width() + kFocusVisibilityTolerance
        && item.height() <= viewport.height() + kFocusVisibilityTolerance
        && item.minX() >= viewport.minX() - kFocusVisibilityTolerance
        && item.maxX() <= viewport.maxX() + kFocusVisibilityTolerance
        && item.minY() >= viewport.minY() - kFocusVisibilityTolerance
        && item.maxY() <= viewport.maxY() + kFocusVisibilityTolerance;
}

NXPoint UIScrollView::focusTrackingTargetOffset(
    const std::shared_ptr<UIView>& view,
    UIScrollViewFocusTrackingMode mode
) {
    auto target = contentOffset();
    if (!view || view.get() == this || !containsFocusView(view)) {
        return getBoundsCheckedContentOffset(target);
    }

    const auto item = focusRectInContent(view);
    const auto viewport = focusViewport(target);
    if (mode == UIScrollViewFocusTrackingMode::centered) {
        target.x += item.midX() - viewport.midX();
        target.y += item.midY() - viewport.midY();
    } else if (mode == UIScrollViewFocusTrackingMode::focused) {
        if (item.minX() < viewport.minX()) {
            target.x += item.minX() - viewport.minX();
        } else if (item.maxX() > viewport.maxX()) {
            target.x += item.maxX() - viewport.maxX();
        }
        if (item.minY() < viewport.minY()) {
            target.y += item.minY() - viewport.minY();
        } else if (item.maxY() > viewport.maxY()) {
            target.y += item.maxY() - viewport.maxY();
        }
    }
    return getBoundsCheckedContentOffset(target);
}

bool UIScrollView::containsFocusView(const std::shared_ptr<UIView>& view) {
    if (!view) {
        return false;
    }
    return view.get() == this
        || view->isDescendantOf(shared_from_base<UIScrollView>());
}

bool UIScrollView::canScrollForFocusHeading(UIFocusHeading heading) {
    if (!_isScrollEnabled) {
        return false;
    }
    const auto limits = contentOffsetBounds();
    const auto offset = contentOffset();
    switch (heading) {
        case UIFocusHeading::up:
            return offset.y > limits.minY() + kFocusVisibilityTolerance;
        case UIFocusHeading::down:
            return offset.y < limits.maxY() - kFocusVisibilityTolerance;
        case UIFocusHeading::left:
            return offset.x > limits.minX() + kFocusVisibilityTolerance;
        case UIFocusHeading::right:
            return offset.x < limits.maxX() - kFocusVisibilityTolerance;
        default:
            return false;
    }
}

bool UIScrollView::shouldUpdateFocusIn(UIFocusUpdateContext context) {
    const auto previous = std::dynamic_pointer_cast<UIView>(
        context.previouslyFocusedItem().lock()
    );
    const auto next = std::dynamic_pointer_cast<UIView>(
        context.nextFocusedItem().lock()
    );
    const auto heading = context.focusHeading();
    if (!isDirectionalFocusHeading(heading) || !containsFocusView(previous)) {
        return true;
    }

    const bool scrollViewIsFocused = previous.get() == this;
    if (!scrollViewIsFocused
        && _focusTrackingMode != UIScrollViewFocusTrackingMode::natural) {
        return true;
    }

    if (next && next != previous && containsFocusView(next)
        && next.get() != this) {
        if (isFullyVisibleForFocus(next)) {
            _naturalPendingFocusView.reset();
            _naturalFocusScrollRequested = false;
            return true;
        }
        _naturalPendingFocusView = next;
    } else {
        _naturalPendingFocusView.reset();
    }

    if (!canScrollForFocusHeading(heading)) {
        _naturalFocusScrollRequested = false;
        return true;
    }

    _naturalFocusScrollRequested = true;
    if (isFocusHeadingPressed(heading)) {
        startNaturalFocusScrollIfNeeded(heading);
    }
    return false;
}

void UIScrollView::didUpdateFocusIn(
    UIFocusUpdateContext context,
    UIFocusAnimationCoordinator* coordinator
) {
    UIView::didUpdateFocusIn(context, coordinator);

    const auto previous = std::dynamic_pointer_cast<UIView>(
        context.previouslyFocusedItem().lock()
    );
    const auto next = std::dynamic_pointer_cast<UIView>(
        context.nextFocusedItem().lock()
    );
    if (containsFocusView(previous) && !containsFocusView(next)) {
        stopNaturalFocusScroll();
        return;
    }
    if (!next || next.get() == this || !containsFocusView(next)
        || _focusTrackingMode == UIScrollViewFocusTrackingMode::natural) {
        return;
    }

    const auto target = focusTrackingTargetOffset(next, _focusTrackingMode);
    const auto weakSelf = weak_from_base<UIScrollView>();
    const auto updateOffset = [weakSelf, target]() {
        if (const auto self = weakSelf.lock()) {
            self->setContentOffset(target, false);
        }
    };
    if (coordinator) {
        coordinator->addCoordinatedAnimations(updateOffset);
    } else {
        updateOffset();
    }
}

bool UIScrollView::isFocusHeadingPressed(UIFocusHeading heading) {
    const auto containingWindow = window();
    return containingWindow && containingWindow->focusSystem()
        && containingWindow->focusSystem()->isFocusHeadingPressed(heading);
}

void UIScrollView::startNaturalFocusScrollIfNeeded(UIFocusHeading heading) {
    const auto focusedView = window() && window()->focusSystem()
        ? std::dynamic_pointer_cast<UIView>(
            window()->focusSystem()->focusedItem().lock()
        )
        : nullptr;
    const bool scrollViewItselfIsFocused = focusedView.get() == this;
    if (!_naturalFocusScrollRequested
        || !isFocusHeadingPressed(heading)
        || (_focusTrackingMode != UIScrollViewFocusTrackingMode::natural
            && !scrollViewItselfIsFocused)
        || !canScrollForFocusHeading(heading)) {
        return;
    }
    if (_naturalFocusScrollActive && _naturalFocusDisplayLink
        && _naturalFocusHeading == heading) {
        return;
    }

    if (_naturalFocusDisplayLink) {
        _naturalFocusDisplayLink->invalidate();
        _naturalFocusDisplayLink.reset();
    }
    const auto visibleOffset = visibleContentOffset();
    cancelDeceleratingIfNeccessary();
    cancelDecelerationAnimations();
    setContentOffset(
        getBoundsCheckedContentOffset(visibleOffset),
        false
    );
    _naturalFocusHeading = heading;
    _naturalFocusScrollActive = true;
    _lastNaturalFocusScrollTimestamp = Timer();
    _naturalFocusDisplayLink = std::make_unique<CADisplayLink>([this]() {
        naturalFocusScrollTick();
    });
}

void UIScrollView::stopNaturalFocusScroll() {
    if (_naturalFocusDisplayLink) {
        _naturalFocusDisplayLink->invalidate();
    }
    _naturalFocusScrollActive = false;
    _naturalFocusHeading = UIFocusHeading::none;
    _naturalFocusScrollRequested = false;
    _naturalPendingFocusView.reset();
}

void UIScrollView::naturalFocusScrollTick() {
    const auto now = Timer();
    const auto elapsedSeconds =
        (now - _lastNaturalFocusScrollTimestamp) / 1000.0;
    _lastNaturalFocusScrollTimestamp = now;
    advanceNaturalFocusScroll(elapsedSeconds);
}

void UIScrollView::advanceNaturalFocusScroll(double elapsedSeconds) {
    if (_naturalFocusHeading == UIFocusHeading::none) {
        return;
    }
    if (!isFocusHeadingPressed(_naturalFocusHeading)
        || !canScrollForFocusHeading(_naturalFocusHeading)) {
        stopNaturalFocusScroll();
        return;
    }
    if (elapsedSeconds <= 0) {
        return;
    }

    CALayer::requestFramerate(120);
    auto remaining = elapsedSeconds;
    constexpr double maximumStep = 1.0 / 120.0;
    while (remaining > 0
        && canScrollForFocusHeading(_naturalFocusHeading)) {
        const auto step = std::min(remaining, maximumStep);
        auto target = contentOffset();
        const auto distance = static_cast<NXFloat>(
            kNaturalFocusScrollSpeed * step
        );
        switch (_naturalFocusHeading) {
            case UIFocusHeading::up:
                target.y -= distance;
                break;
            case UIFocusHeading::down:
                target.y += distance;
                break;
            case UIFocusHeading::left:
                target.x -= distance;
                break;
            case UIFocusHeading::right:
                target.x += distance;
                break;
            default:
                return;
        }
        setContentOffset(getBoundsCheckedContentOffset(target), false);
        handOffNaturalFocusIfPossible();
        remaining -= step;
    }

    if (!canScrollForFocusHeading(_naturalFocusHeading)) {
        stopNaturalFocusScroll();
    }
}

void UIScrollView::handOffNaturalFocusIfPossible() {
    const auto containingWindow = window();
    if (!containingWindow || !containingWindow->focusSystem()) {
        stopNaturalFocusScroll();
        return;
    }
    auto current = std::dynamic_pointer_cast<UIView>(
        containingWindow->focusSystem()->focusedItem().lock()
    );
    if (!containsFocusView(current)) {
        stopNaturalFocusScroll();
        return;
    }

    auto pending = _naturalPendingFocusView.lock();
    if (!pending && current.get() != this) {
        pending = nextFocusableDescendant(current, _naturalFocusHeading);
        _naturalPendingFocusView = pending;
    }
    if (pending && isFullyVisibleForFocus(pending)) {
        _naturalPendingFocusView.reset();
        containingWindow->focusSystem()->requestFocusUpdate(pending);
        current = pending;
    } else if (current.get() != this && !isFullyVisibleForFocus(current)) {
        containingWindow->focusSystem()->requestExactFocusUpdate(
            shared_from_base<UIScrollView>()
        );
        current = shared_from_base<UIScrollView>();
    }

}

void UIScrollView::setFocusTrackingMode(
    UIScrollViewFocusTrackingMode mode
) {
    switch (mode) {
        case UIScrollViewFocusTrackingMode::natural:
        case UIScrollViewFocusTrackingMode::centered:
        case UIScrollViewFocusTrackingMode::focused:
            break;
        default:
            throw std::invalid_argument(
                "UIScrollView focus tracking mode is invalid"
            );
    }
    if (_focusTrackingMode == mode) {
        return;
    }

    stopNaturalFocusScroll();
    _focusTrackingMode = mode;
    if (mode == UIScrollViewFocusTrackingMode::natural) {
        return;
    }

    const auto containingWindow = window();
    const auto focusedView = containingWindow && containingWindow->focusSystem()
        ? std::dynamic_pointer_cast<UIView>(
            containingWindow->focusSystem()->focusedItem().lock()
        )
        : nullptr;
    if (!focusedView || focusedView.get() == this
        || !containsFocusView(focusedView)) {
        return;
    }
    const auto target = focusTrackingTargetOffset(focusedView, mode);
    const auto weakSelf = weak_from_base<UIScrollView>();
    UIView::animate(
        0.2,
        0,
        UIViewAnimationOptions(
            beginFromCurrentState | curveEaseOut | allowUserInteraction
        ),
        [weakSelf, target]() {
            if (const auto self = weakSelf.lock()) {
                self->setContentOffset(target, false);
            }
        }
    );
}

void UIScrollView::addSubview(const std::shared_ptr<UIView> &view) {
    UIView::addSubview(view);
    // view->yoga->setPositionType(YGPositionTypeAbsolute);
}

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

void UIScrollView::setScrollEnabled(bool scrollEnabled) {
    if (_isScrollEnabled == scrollEnabled) return;

    _isScrollEnabled = scrollEnabled;
    _panGestureRecognizer->setEnabled(scrollEnabled);
    if (scrollEnabled) return;

    if (_timerAnimation) {
        _timerAnimation->invalidate();
    }
    stopNaturalFocusScroll();
    cancelDecelerationAnimations();
    _isDecelerating = false;
    weightedAverageVelocity = NXPoint();
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

void UIScrollView::safeAreaInsetsDidChange() {
    const auto delta = _lastSafeAreaInsets - safeAreaInsets();
    _lastSafeAreaInsets = safeAreaInsets();
    const auto target = getBoundsCheckedContentOffset(contentOffset() + NXPoint(delta.left, delta.top));
    setContentOffset(target, false);
}

NXPoint UIScrollView::visibleContentOffset() const {
    return (layer()->presentationOrSelf())->bounds().origin;
}

NXSize UIScrollView::contentSize() const {
    if (_hasExplicitContentSize) return _contentSize;

    // Preserve the historical inferred behavior for existing NXKit callers.
    // New code should set contentSize explicitly, matching UIKit.
    if (subviews().empty()) return {};
    return subviews().front()->bounds().size;
}

void UIScrollView::setContentSize(NXSize contentSize) {
    if (!std::isfinite(contentSize.width)
        || !std::isfinite(contentSize.height)
        || contentSize.width < 0
        || contentSize.height < 0) {
        throw std::invalid_argument(
            "UIScrollView content size must be finite and non-negative"
        );
    }
    if (_hasExplicitContentSize && _contentSize == contentSize) return;

    _contentSize = contentSize;
    _hasExplicitContentSize = true;
    setNeedsLayout();
}

UIEdgeInsets UIScrollView::adjustedContentInset() {
    return effectiveContentInsets();
}

UIEdgeInsets UIScrollView::effectiveContentInsets() {
    auto allInsets = _contentInset;
    auto automaticInsets = safeAreaInsets();

    const bool contentWidthGreaterThanScrollBounds = contentSize().width > bounds().width() - _contentInset.left - _contentInset.right;
    const bool contentHeightGreaterThanScrollBounds = contentSize().height > bounds().height() - _contentInset.top - _contentInset.bottom;

    switch (_contentInsetAdjustmentBehavior) {
        case UIScrollViewContentInsetAdjustmentBehavior::scrollableAxes: {
            if (contentWidthGreaterThanScrollBounds || _bounceHorizontally) {
                allInsets += UIEdgeInsets(0, automaticInsets.left, 0, automaticInsets.right);
            }
            if (contentHeightGreaterThanScrollBounds || _bounceVertically) {
                allInsets += UIEdgeInsets(automaticInsets.top, 0, automaticInsets.bottom, 0);
            }
            break;
        }
        case UIScrollViewContentInsetAdjustmentBehavior::always: {
            allInsets += automaticInsets;
            break;
        }
        case UIScrollViewContentInsetAdjustmentBehavior::never: {
            break;
        }
    }

    return allInsets;
}

NXPoint UIScrollView::getBoundsCheckedContentOffset(NXPoint newContentOffset) {
    auto contentNXSize = this->contentSize();
    auto contentHeight = contentNXSize.height;// fmaxf(contentNXSize.height, bounds().height());
    auto contentWidth = contentNXSize.width;// fmaxf(contentNXSize.width, bounds().width());

    auto allInsects = effectiveContentInsets();

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

    auto allInsects = effectiveContentInsets();

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

bool UIScrollView::shouldBounceHorizontally() const {
    return _bounceHorizontally;
}

bool UIScrollView::shouldBounceVertically() const {
    return _bounceVertically;
}

void UIScrollView::onPan() {
    const auto translation = _panGestureRecognizer->translationInView(shared_from_this());
//    _panGestureRecognizer->setTranslation(NXPoint(), shared_from_this());

    const auto panGestureVelocity = _panGestureRecognizer->velocityIn(shared_from_this());
    weightedAverageVelocity = weightedAverageVelocity * 0.2 + panGestureVelocity * 0.8;

    // auto offset = contentOffsetBounds();
    const auto rubberBand = RubberBand(0.55f, frame().size, contentOffsetBounds());

    const NXPoint clamped = getBoundsCheckedContentOffset(_initialContentOffset - translation);
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
    auto velocity = weightedAverageVelocity * -1.0f;
    constexpr NXFloat maximumDecelerationVelocity = 12000.0f;
    velocity.x = std::clamp(velocity.x, -maximumDecelerationVelocity, maximumDecelerationVelocity);
    velocity.y = std::clamp(velocity.y, -maximumDecelerationVelocity, maximumDecelerationVelocity);

    const auto scrollableOffsetBounds = contentOffsetBounds();
    if (scrollableOffsetBounds.height() <= 0 && !shouldBounceVertically()) {
        velocity.y = 0;
    }
    if (scrollableOffsetBounds.width() <= 0 && !shouldBounceHorizontally()) {
        velocity.x = 0;
    }

    auto decelerationRate = _decelerationRate.rawValue();
    auto threshold = 0.5f / traitCollection()->displayScale();// layer()->contentsScale();

    auto parameters = DecelerationTimingParameters(contentOffset(),velocity, decelerationRate, threshold);

    auto destination = parameters.destination();
    auto clippedDestination = getBoundsCheckedContentOffset(destination);
    const bool isHorizontallyClipped = destination.x != clippedDestination.x;
    const bool isVerticallyClipped = destination.y != clippedDestination.y;
    const bool isClipped = isHorizontallyClipped || isVerticallyClipped;
    const bool shouldBounceAtDestination =
        (isHorizontallyClipped && shouldBounceHorizontally())
        || (isVerticallyClipped && shouldBounceVertically());

    float duration;
    if (isClipped) {
        duration = parameters.durationTo(clippedDestination);
    } else {
        duration = parameters.duration();
    }

    _isDecelerating = true;
    _timerAnimation = std::make_shared<TimerAnimation>(duration, [this, parameters](float, const double time) {
        setContentOffset(parameters.valueAt(time), false);
    }, [
        this,
        parameters,
        duration,
        clippedDestination,
        shouldBounceAtDestination
    ](bool) {
        auto settledOffset = contentOffset();
        if (!shouldBounceHorizontally()) {
            settledOffset.x = clippedDestination.x;
        }
        if (!shouldBounceVertically()) {
            settledOffset.y = clippedDestination.y;
        }
        setContentOffset(settledOffset, false);

        _isDecelerating = shouldBounceAtDestination;

        if (shouldBounceAtDestination) {
            auto velocity = parameters.velocityAt(duration);
            if (!shouldBounceHorizontally()) {
                velocity.x = 0;
            }
            if (!shouldBounceVertically()) {
                velocity.y = 0;
            }
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
        CALayer::requestFramerate(120);
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

bool UIScrollView::applyXMLAttribute(const std::string& name, const std::string& value) {
    if (UIView::applyXMLAttribute(name, value)) return true;

    if (name == "focusTrackingMode") {
        if (value == "natural") {
            setFocusTrackingMode(UIScrollViewFocusTrackingMode::natural);
            return true;
        }
        if (value == "centered") {
            setFocusTrackingMode(UIScrollViewFocusTrackingMode::centered);
            return true;
        }
        if (value == "focused") {
            setFocusTrackingMode(UIScrollViewFocusTrackingMode::focused);
            return true;
        }
        return false;
    }

    REGISTER_XIB_ATTRIBUTE(scrollEnabled, valueToBool, setScrollEnabled)
    REGISTER_XIB_ATTRIBUTE(bounceVertically, valueToBool, setBounceVertically)
    REGISTER_XIB_ATTRIBUTE(bounceHorizontally, valueToBool, setBounceHorizontally)

    return false;
}

void UIScrollView::layoutSubviews() {
    UIView::layoutSubviews();
    setContentOffset(getBoundsCheckedContentOffset(contentOffset()), false);

    if (_focusTrackingMode == UIScrollViewFocusTrackingMode::natural) {
        return;
    }
    const auto containingWindow = window();
    const auto focusedView = containingWindow && containingWindow->focusSystem()
        ? std::dynamic_pointer_cast<UIView>(
            containingWindow->focusSystem()->focusedItem().lock()
        )
        : nullptr;
    if (focusedView && focusedView.get() != this
        && containsFocusView(focusedView)) {
        setContentOffset(
            focusTrackingTargetOffset(focusedView, _focusTrackingMode),
            false
        );
    }
}

}
