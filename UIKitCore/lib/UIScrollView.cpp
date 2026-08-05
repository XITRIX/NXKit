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
#include <DispatchQueue.h>
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
constexpr NXFloat kScrollIndicatorThickness = 3.0f;
constexpr NXFloat kScrollIndicatorEdgePadding = 2.0f;
constexpr NXFloat kScrollIndicatorMinimumLength = 36.0f;
constexpr NXFloat kScrollIndicatorZPosition = 1000000.0f;
constexpr double kScrollIndicatorFlashDuration = 0.5;
constexpr double kScrollIndicatorFadeDuration = 0.25;

struct ScrollIndicatorAxisGeometry {
    NXFloat origin = 0;
    NXFloat length = 0;
    bool visible = false;
};

bool finiteInsets(const UIEdgeInsets& insets) {
    return std::isfinite(insets.top)
        && std::isfinite(insets.left)
        && std::isfinite(insets.bottom)
        && std::isfinite(insets.right);
}

ScrollIndicatorAxisGeometry scrollIndicatorAxisGeometry(
    NXFloat trackOrigin,
    NXFloat trackLength,
    NXFloat viewportLength,
    NXFloat scrollRange,
    NXFloat relativeContentOffset
) {
    if (trackLength <= 0 || viewportLength <= 0 || scrollRange <= 0) {
        return {};
    }

    const auto totalScrollableLength = viewportLength + scrollRange;
    const auto proportionalLength =
        trackLength * viewportLength / totalScrollableLength;
    const auto normalLength = std::min(
        trackLength,
        std::max(
            std::min(kScrollIndicatorMinimumLength, trackLength),
            proportionalLength
        )
    );

    const auto leadingOverscroll = std::max<NXFloat>(
        0,
        -relativeContentOffset
    );
    const auto trailingOverscroll = std::max<NXFloat>(
        0,
        relativeContentOffset - scrollRange
    );
    const auto overscroll = leadingOverscroll + trailingOverscroll;
    const auto minimumCompressedLength = std::min(
        kScrollIndicatorThickness,
        normalLength
    );
    const auto indicatorLength = std::max(
        minimumCompressedLength,
        normalLength - overscroll
    );

    NXFloat indicatorOrigin;
    if (leadingOverscroll > 0) {
        indicatorOrigin = trackOrigin;
    } else if (trailingOverscroll > 0) {
        indicatorOrigin = trackOrigin + trackLength - indicatorLength;
    } else {
        const auto progress = std::clamp(
            relativeContentOffset / scrollRange,
            NXFloat(0),
            NXFloat(1)
        );
        indicatorOrigin = trackOrigin
            + (trackLength - normalLength) * progress;
    }

    return {indicatorOrigin, indicatorLength, true};
}

UIColor scrollIndicatorColor(UIScrollViewIndicatorStyle style) {
    switch (style) {
        case UIScrollViewIndicatorStyle::defaultStyle:
            return UIColor::label.withAlphaComponent(0.5f);
        case UIScrollViewIndicatorStyle::black:
            return UIColor::black.withAlphaComponent(0.5f);
        case UIScrollViewIndicatorStyle::white:
            return UIColor::white.withAlphaComponent(0.6f);
    }
    return UIColor::label.withAlphaComponent(0.5f);
}

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

    _horizontalScrollIndicatorLayer = new_shared<CALayer>();
    _verticalScrollIndicatorLayer = new_shared<CALayer>();
    for (const auto& indicatorLayer : {
        _horizontalScrollIndicatorLayer,
        _verticalScrollIndicatorLayer
    }) {
        indicatorLayer->setZPosition(kScrollIndicatorZPosition);
        indicatorLayer->setMasksToBounds(true);
        indicatorLayer->setOpacity(0);
        indicatorLayer->setHidden(true);
        layer()->addSublayer(indicatorLayer);
    }
    applyScrollIndicatorStyle();
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
    showScrollIndicators();
    _lastNaturalFocusScrollTimestamp = Timer();
    _naturalFocusDisplayLink = std::make_unique<CADisplayLink>([this]() {
        naturalFocusScrollTick();
    });
}

void UIScrollView::stopNaturalFocusScroll() {
    const bool wasActive = _naturalFocusScrollActive;
    if (_naturalFocusDisplayLink) {
        _naturalFocusDisplayLink->invalidate();
    }
    _naturalFocusScrollActive = false;
    _naturalFocusHeading = UIFocusHeading::none;
    _naturalFocusScrollRequested = false;
    _naturalPendingFocusView.reset();
    if (wasActive) {
        hideScrollIndicators();
    }
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
    // UIView's indexed layer insertion assumes that view-backed layers occupy
    // the leading positions in the backing layer. Keep private overlays after
    // them so UIScrollView does not disturb ordinary subview ordering.
    _horizontalScrollIndicatorLayer->removeFromSuperlayer();
    _verticalScrollIndicatorLayer->removeFromSuperlayer();
    UIView::addSubview(view);
    layer()->addSublayer(_horizontalScrollIndicatorLayer);
    layer()->addSublayer(_verticalScrollIndicatorLayer);
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
    hideScrollIndicators();
}

void UIScrollView::setContentInset(UIEdgeInsets contentInset) {
    if (!finiteInsets(contentInset)) {
        throw std::invalid_argument(
            "UIScrollView content inset must be finite"
        );
    }
    if (_contentInset == contentInset) return;

    _contentInset = contentInset;
    setNeedsLayout();
}

void UIScrollView::setShowsVerticalScrollIndicator(
    bool showsVerticalScrollIndicator
) {
    if (_showsVerticalScrollIndicator == showsVerticalScrollIndicator) {
        return;
    }
    _showsVerticalScrollIndicator = showsVerticalScrollIndicator;
    layoutScrollIndicatorsIfNeeded();
}

void UIScrollView::setShowsHorizontalScrollIndicator(
    bool showsHorizontalScrollIndicator
) {
    if (_showsHorizontalScrollIndicator == showsHorizontalScrollIndicator) {
        return;
    }
    _showsHorizontalScrollIndicator = showsHorizontalScrollIndicator;
    layoutScrollIndicatorsIfNeeded();
}

void UIScrollView::setIndicatorStyle(
    UIScrollViewIndicatorStyle indicatorStyle
) {
    switch (indicatorStyle) {
        case UIScrollViewIndicatorStyle::defaultStyle:
        case UIScrollViewIndicatorStyle::black:
        case UIScrollViewIndicatorStyle::white:
            break;
        default:
            throw std::invalid_argument(
                "UIScrollView indicator style is invalid"
            );
    }
    if (_indicatorStyle == indicatorStyle) return;

    _indicatorStyle = indicatorStyle;
    applyScrollIndicatorStyle();
}

void UIScrollView::setVerticalScrollIndicatorInsets(UIEdgeInsets insets) {
    if (!finiteInsets(insets)) {
        throw std::invalid_argument(
            "UIScrollView vertical scroll indicator insets must be finite"
        );
    }
    if (_verticalScrollIndicatorInsets == insets) return;

    _verticalScrollIndicatorInsets = insets;
    layoutScrollIndicatorsIfNeeded();
}

void UIScrollView::setHorizontalScrollIndicatorInsets(UIEdgeInsets insets) {
    if (!finiteInsets(insets)) {
        throw std::invalid_argument(
            "UIScrollView horizontal scroll indicator insets must be finite"
        );
    }
    if (_horizontalScrollIndicatorInsets == insets) return;

    _horizontalScrollIndicatorInsets = insets;
    layoutScrollIndicatorsIfNeeded();
}

void UIScrollView::setScrollIndicatorInsets(UIEdgeInsets insets) {
    if (!finiteInsets(insets)) {
        throw std::invalid_argument(
            "UIScrollView scroll indicator insets must be finite"
        );
    }
    if (_verticalScrollIndicatorInsets == insets
        && _horizontalScrollIndicatorInsets == insets) {
        return;
    }

    _verticalScrollIndicatorInsets = insets;
    _horizontalScrollIndicatorInsets = insets;
    layoutScrollIndicatorsIfNeeded();
}

void UIScrollView::flashScrollIndicators() {
    showScrollIndicators();
    scheduleScrollIndicatorHide(kScrollIndicatorFlashDuration);
}

void UIScrollView::withScrollIndicatorsShownForContentOffsetChanges(
    const std::function<void()>& changes
) {
    if (!changes) {
        throw std::invalid_argument(
            "UIScrollView content-offset changes block must not be empty"
        );
    }

    const auto initialOffset = contentOffset();
    changes();
    const auto finalOffset = contentOffset();
    const bool changedHorizontally = initialOffset.x != finalOffset.x;
    const bool changedVertically = initialOffset.y != finalOffset.y;
    if (!changedHorizontally && !changedVertically) return;

    showScrollIndicators(changedHorizontally, changedVertically);
    scheduleScrollIndicatorHide(kScrollIndicatorFlashDuration);
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
    layoutScrollIndicatorsIfNeeded();
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
            cancelDeceleratingIfNeccessary();
            break;
        }
        case UIGestureRecognizerState::began: {
            _initialContentOffset = contentOffset();
            showScrollIndicators();
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
        case UIGestureRecognizerState::cancelled:
        case UIGestureRecognizerState::failed: {
            hideScrollIndicators();
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
    const auto viewport = bounds();
    const auto safeArea = safeAreaInsets();
    const auto horizontalInsets = UIEdgeInsets(
        std::max(_horizontalScrollIndicatorInsets.top, safeArea.top),
        std::max(_horizontalScrollIndicatorInsets.left, safeArea.left),
        std::max(_horizontalScrollIndicatorInsets.bottom, safeArea.bottom),
        std::max(_horizontalScrollIndicatorInsets.right, safeArea.right)
    );
    const auto verticalInsets = UIEdgeInsets(
        std::max(_verticalScrollIndicatorInsets.top, safeArea.top),
        std::max(_verticalScrollIndicatorInsets.left, safeArea.left),
        std::max(_verticalScrollIndicatorInsets.bottom, safeArea.bottom),
        std::max(_verticalScrollIndicatorInsets.right, safeArea.right)
    );
    const auto offsetBounds = contentOffsetBounds();
    const bool horizontalScrollable = offsetBounds.width() > 0;
    const bool verticalScrollable = offsetBounds.height() > 0;
    const bool horizontalHasVisibleCrossAxisSpace =
        viewport.height()
            - horizontalInsets.top
            - horizontalInsets.bottom
        >= kScrollIndicatorThickness + kScrollIndicatorEdgePadding * 2;
    const bool verticalHasVisibleCrossAxisSpace =
        viewport.width()
            - verticalInsets.left
            - verticalInsets.right
        >= kScrollIndicatorThickness + kScrollIndicatorEdgePadding * 2;
    const bool laysOutHorizontal = _showsHorizontalScrollIndicator
        && horizontalScrollable
        && horizontalHasVisibleCrossAxisSpace;
    const bool laysOutVertical = _showsVerticalScrollIndicator
        && verticalScrollable
        && verticalHasVisibleCrossAxisSpace;

    auto horizontalTrackOrigin = horizontalInsets.left
        + kScrollIndicatorEdgePadding;
    auto horizontalTrackLength = viewport.width()
        - horizontalInsets.left
        - horizontalInsets.right
        - kScrollIndicatorEdgePadding * 2;
    if (laysOutVertical) {
        horizontalTrackLength -=
            kScrollIndicatorThickness + kScrollIndicatorEdgePadding;
    }

    auto verticalTrackOrigin = verticalInsets.top
        + kScrollIndicatorEdgePadding;
    auto verticalTrackLength = viewport.height()
        - verticalInsets.top
        - verticalInsets.bottom
        - kScrollIndicatorEdgePadding * 2;
    if (laysOutHorizontal) {
        verticalTrackLength -=
            kScrollIndicatorThickness + kScrollIndicatorEdgePadding;
    }

    const auto horizontalGeometry = scrollIndicatorAxisGeometry(
        horizontalTrackOrigin,
        horizontalTrackLength,
        viewport.width(),
        offsetBounds.width(),
        contentOffset().x - offsetBounds.origin.x
    );
    const auto verticalGeometry = scrollIndicatorAxisGeometry(
        verticalTrackOrigin,
        verticalTrackLength,
        viewport.height(),
        offsetBounds.height(),
        contentOffset().y - offsetBounds.origin.y
    );

    const bool horizontalVisible = laysOutHorizontal
        && horizontalGeometry.visible;
    const bool verticalVisible = laysOutVertical
        && verticalGeometry.visible;

    CATransaction::begin();
    CATransaction::setDisableActions(true);

    _horizontalScrollIndicatorLayer->setFrame({
        viewport.origin.x + horizontalGeometry.origin,
        viewport.origin.y + viewport.height()
            - horizontalInsets.bottom
            - kScrollIndicatorEdgePadding
            - kScrollIndicatorThickness,
        horizontalGeometry.length,
        kScrollIndicatorThickness
    });
    _horizontalScrollIndicatorLayer->setCornerRadius(
        kScrollIndicatorThickness / 2
    );
    if (_horizontalScrollIndicatorLayer->isHidden() == horizontalVisible) {
        _horizontalScrollIndicatorLayer->setHidden(!horizontalVisible);
    }

    _verticalScrollIndicatorLayer->setFrame({
        viewport.origin.x + viewport.width()
            - verticalInsets.right
            - kScrollIndicatorEdgePadding
            - kScrollIndicatorThickness,
        viewport.origin.y + verticalGeometry.origin,
        kScrollIndicatorThickness,
        verticalGeometry.length
    });
    _verticalScrollIndicatorLayer->setCornerRadius(
        kScrollIndicatorThickness / 2
    );
    if (_verticalScrollIndicatorLayer->isHidden() == verticalVisible) {
        _verticalScrollIndicatorLayer->setHidden(!verticalVisible);
    }

    CATransaction::commit();
}

void UIScrollView::applyScrollIndicatorStyle() {
    CATransaction::begin();
    CATransaction::setDisableActions(true);
    const auto color = scrollIndicatorColor(_indicatorStyle);
    _horizontalScrollIndicatorLayer->setBackgroundColor(color);
    _verticalScrollIndicatorLayer->setBackgroundColor(color);
    CATransaction::commit();
}

void UIScrollView::showScrollIndicators() {
    showScrollIndicators(true, true);
}

void UIScrollView::showScrollIndicators(bool horizontal, bool vertical) {
    if (!horizontal && !vertical) return;

    ++_scrollIndicatorVisibilityGeneration;
    _scrollIndicatorsShown = true;
    layoutScrollIndicatorsIfNeeded();

    CATransaction::begin();
    CATransaction::setDisableActions(true);
    if (horizontal) {
        _horizontalScrollIndicatorLayer->removeAnimation("opacity");
        _horizontalScrollIndicatorLayer->setOpacity(1);
    }
    if (vertical) {
        _verticalScrollIndicatorLayer->removeAnimation("opacity");
        _verticalScrollIndicatorLayer->setOpacity(1);
    }
    CATransaction::commit();
}

void UIScrollView::hideScrollIndicators() {
    ++_scrollIndicatorVisibilityGeneration;
    _scrollIndicatorsShown = false;

    for (const auto& indicatorLayer : {
        _horizontalScrollIndicatorLayer,
        _verticalScrollIndicatorLayer
    }) {
        if (indicatorLayer->isHidden()
            || indicatorLayer->presentationOrSelf()->opacity() <= 0) {
            continue;
        }
        auto fade = new_shared<CABasicAnimation>("opacity");
        fade->duration = kScrollIndicatorFadeDuration;
        indicatorLayer->add(fade, "opacity");
    }

    CATransaction::begin();
    CATransaction::setDisableActions(true);
    _horizontalScrollIndicatorLayer->setOpacity(0);
    _verticalScrollIndicatorLayer->setOpacity(0);
    CATransaction::commit();
}

void UIScrollView::scheduleScrollIndicatorHide(double delay) {
    const auto generation = _scrollIndicatorVisibilityGeneration;
    const auto weakSelf = weak_from_base<UIScrollView>();
    DispatchQueue::main()->asyncAfter(delay, [weakSelf, generation]() {
        if (const auto self = weakSelf.lock(); self
            && self->_scrollIndicatorVisibilityGeneration == generation
            && self->_scrollIndicatorsShown) {
            self->hideScrollIndicators();
        }
    });
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
        } else {
            hideScrollIndicators();
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
        hideScrollIndicators();
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

    if (name == "indicatorStyle") {
        if (value == "default") {
            setIndicatorStyle(UIScrollViewIndicatorStyle::defaultStyle);
            return true;
        }
        if (value == "black") {
            setIndicatorStyle(UIScrollViewIndicatorStyle::black);
            return true;
        }
        if (value == "white") {
            setIndicatorStyle(UIScrollViewIndicatorStyle::white);
            return true;
        }
        return false;
    }

    REGISTER_XIB_ATTRIBUTE(scrollEnabled, valueToBool, setScrollEnabled)
    REGISTER_XIB_ATTRIBUTE(bounceVertically, valueToBool, setBounceVertically)
    REGISTER_XIB_ATTRIBUTE(bounceHorizontally, valueToBool, setBounceHorizontally)
    REGISTER_XIB_ATTRIBUTE(
        showsVerticalScrollIndicator,
        valueToBool,
        setShowsVerticalScrollIndicator
    )
    REGISTER_XIB_ATTRIBUTE(
        showsHorizontalScrollIndicator,
        valueToBool,
        setShowsHorizontalScrollIndicator
    )

    return false;
}

void UIScrollView::layoutSubviews() {
    UIView::layoutSubviews();
    setContentOffset(getBoundsCheckedContentOffset(contentOffset()), false);
    layoutScrollIndicatorsIfNeeded();

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
