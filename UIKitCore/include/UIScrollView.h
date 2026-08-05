//
//  UIScrollView.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 12.03.2023.
//

#pragma once

#include <UIView.h>
#include <CADisplayLink.h>
#include <UIPanGestureRecognizer.h>
#include <UIScrollViewExtensions/DecelerationTimingParameters.h>
#include <UIScrollViewExtensions/TimerAnimation.h>
#include <UIScrollViewExtensions/UIScrollViewDecelerationRate.h>
#include <cstdint>
#include <functional>

namespace NXKit {

class UIScrollView;
class UIScrollViewDelegate {
public:
    virtual void scrollViewWillBeginDragging(std::shared_ptr<UIScrollView> scrollView) {}
    virtual void scrollViewDidScroll(std::shared_ptr<UIScrollView> scrollView) {}
    virtual void scrollViewDidEndDragging(std::shared_ptr<UIScrollView> scrollView, bool willDecelerate) {}
};

enum class UIScrollViewContentInsetAdjustmentBehavior {
    scrollableAxes,
    never,
    always
};

enum class UIScrollViewIndicatorStyle {
    // C++ cannot use UIKit's `default` case name because it is a keyword.
    defaultStyle,
    black,
    white
};

// A portable extension controlling how controller/keyboard focus tracks
// scrollable content. UIKit does not expose an equivalent three-mode API.
enum class UIScrollViewFocusTrackingMode {
    // Visit fully visible items first, then scroll at a constant rate and move
    // focus whenever the next item becomes fully visible.
    natural,
    // Update focus normally and keep the focused item as close to the viewport
    // center as the scroll limits allow.
    centered,
    // Update focus normally and scroll only when needed to make the focused
    // item fully visible.
    focused
};

class UIScrollViewFocusTestHarness;
class UIScrollViewIndicatorTestHarness;

class UIScrollView: public UIView {
public:
    static std::shared_ptr<UIView> init() { return new_shared<UIScrollView>(); }

    std::weak_ptr<UIScrollViewDelegate> delegate;

    UIScrollView(NXRect frame = NXRect());
    ~UIScrollView() override;

    void addSubview(const std::shared_ptr<UIView> &view) override;
    bool applyXMLAttribute(const std::string& name, const std::string& value) override;
    void layoutSubviews() override;

    bool canBecomeFocused() override;
    bool shouldUpdateFocusIn(UIFocusUpdateContext context) override;
    void didUpdateFocusIn(
        UIFocusUpdateContext context,
        UIFocusAnimationCoordinator* coordinator
    ) override;

    void safeAreaInsetsDidChange() override;

    NXPoint contentOffset() { return bounds().origin; }
    virtual void setContentOffset(NXPoint offset, bool animated);

    [[nodiscard]] bool isScrollEnabled() const { return _isScrollEnabled; }
    void setScrollEnabled(bool scrollEnabled);

    UIEdgeInsets contentInset() { return _contentInset; }
    void setContentInset(UIEdgeInsets contentInset);

    bool bounceHorizontally() const { return _bounceHorizontally; }
    void setBounceHorizontally(bool bounceHorizontally);

    bool bounceVertically() const { return _bounceVertically; }
    void setBounceVertically(bool bounceVertically);

    UIScrollViewDecelerationRate decelerationRate() const { return _decelerationRate; }
    void setDecelerationRate(UIScrollViewDecelerationRate decelerationRate) { _decelerationRate = decelerationRate; }
    [[nodiscard]] bool isDecelerating() const { return _isDecelerating; }

    [[nodiscard]] bool showsVerticalScrollIndicator() const {
        return _showsVerticalScrollIndicator;
    }
    void setShowsVerticalScrollIndicator(bool showsVerticalScrollIndicator);

    [[nodiscard]] bool showsHorizontalScrollIndicator() const {
        return _showsHorizontalScrollIndicator;
    }
    void setShowsHorizontalScrollIndicator(bool showsHorizontalScrollIndicator);

    [[nodiscard]] UIScrollViewIndicatorStyle indicatorStyle() const {
        return _indicatorStyle;
    }
    void setIndicatorStyle(UIScrollViewIndicatorStyle indicatorStyle);

    [[nodiscard]] UIEdgeInsets verticalScrollIndicatorInsets() const {
        return _verticalScrollIndicatorInsets;
    }
    void setVerticalScrollIndicatorInsets(UIEdgeInsets insets);

    [[nodiscard]] UIEdgeInsets horizontalScrollIndicatorInsets() const {
        return _horizontalScrollIndicatorInsets;
    }
    void setHorizontalScrollIndicatorInsets(UIEdgeInsets insets);

    [[deprecated(
        "Use verticalScrollIndicatorInsets() and "
        "horizontalScrollIndicatorInsets() instead"
    )]]
    [[nodiscard]] UIEdgeInsets scrollIndicatorInsets() const {
        return _verticalScrollIndicatorInsets;
    }
    void setScrollIndicatorInsets(UIEdgeInsets insets);

    void flashScrollIndicators();
    void withScrollIndicatorsShownForContentOffsetChanges(
        const std::function<void()>& changes
    );

    UIScrollViewContentInsetAdjustmentBehavior contentInsetAdjustmentBehavior() { return _contentInsetAdjustmentBehavior; }
    void setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior contentInsetAdjustmentBehavior);

    NXSize contentSize() const;
    void setContentSize(NXSize contentSize);

    UIEdgeInsets adjustedContentInset();

    [[nodiscard]] UIScrollViewFocusTrackingMode focusTrackingMode() const {
        return _focusTrackingMode;
    }
    void setFocusTrackingMode(UIScrollViewFocusTrackingMode mode);

private:
    std::shared_ptr<UIPanGestureRecognizer> _panGestureRecognizer;
    bool _isDecelerating = false;
    NXPoint weightedAverageVelocity;
    NXPoint _initialContentOffset;
    UIScrollViewDecelerationRate _decelerationRate = UIScrollViewDecelerationRate::normal;

    bool _bounceHorizontally = false;
    bool _bounceVertically = false;
    bool _isScrollEnabled = true;

    bool shouldBounceHorizontally() const;
    bool shouldBounceVertically() const;

    UIScrollViewContentInsetAdjustmentBehavior _contentInsetAdjustmentBehavior = UIScrollViewContentInsetAdjustmentBehavior::scrollableAxes;

    std::shared_ptr<TimerAnimation> _timerAnimation;
    UIEdgeInsets _lastSafeAreaInsets;
    UIEdgeInsets _contentInset;
    NXSize _contentSize;
    bool _hasExplicitContentSize = false;

    bool _showsVerticalScrollIndicator = true;
    bool _showsHorizontalScrollIndicator = true;
    UIScrollViewIndicatorStyle _indicatorStyle =
        UIScrollViewIndicatorStyle::defaultStyle;
    UIEdgeInsets _verticalScrollIndicatorInsets;
    UIEdgeInsets _horizontalScrollIndicatorInsets;
    std::shared_ptr<CALayer> _verticalScrollIndicatorLayer;
    std::shared_ptr<CALayer> _horizontalScrollIndicatorLayer;
    bool _scrollIndicatorsShown = false;
    std::uint64_t _scrollIndicatorVisibilityGeneration = 0;

    UIScrollViewFocusTrackingMode _focusTrackingMode =
        UIScrollViewFocusTrackingMode::natural;
    std::unique_ptr<CADisplayLink> _naturalFocusDisplayLink;
    Timer _lastNaturalFocusScrollTimestamp;
    UIFocusHeading _naturalFocusHeading = UIFocusHeading::none;
    std::weak_ptr<UIView> _naturalPendingFocusView;
    bool _naturalFocusScrollRequested = false;
    bool _naturalFocusScrollActive = false;

    UIEdgeInsets effectiveContentInsets();

    std::shared_ptr<UIFocusItem> searchForFocus() override;
    std::shared_ptr<UIView> findFocusableDescendant(bool visibleOnly);
    std::shared_ptr<UIView> nextFocusableDescendant(
        const std::shared_ptr<UIView>& current,
        UIFocusHeading heading
    );
    NXRect focusRectInContent(const std::shared_ptr<UIView>& view);
    NXRect focusViewport(NXPoint offset);
    bool isFullyVisibleForFocus(const std::shared_ptr<UIView>& view);
    NXPoint focusTrackingTargetOffset(
        const std::shared_ptr<UIView>& view,
        UIScrollViewFocusTrackingMode mode
    );
    bool containsFocusView(const std::shared_ptr<UIView>& view);
    bool canScrollForFocusHeading(UIFocusHeading heading);
    bool isFocusHeadingPressed(UIFocusHeading heading);
    void startNaturalFocusScrollIfNeeded(UIFocusHeading heading);
    void stopNaturalFocusScroll();
    void naturalFocusScrollTick();
    void advanceNaturalFocusScroll(double elapsedSeconds);
    void handOffNaturalFocusIfPossible();
    void onPan();
    void onPanGestureStateChanged();

    NXPoint visibleContentOffset() const;
    NXPoint getBoundsCheckedContentOffset(NXPoint newContentOffset);

    NXRect contentOffsetBounds();

    void layoutScrollIndicatorsIfNeeded();
    void applyScrollIndicatorStyle();
    void showScrollIndicators();
    void showScrollIndicators(bool horizontal, bool vertical);
    void hideScrollIndicators();
    void scheduleScrollIndicatorHide(double delay);

    void startDeceleratingIfNecessary();
    void cancelDeceleratingIfNeccessary();
    void cancelDecelerationAnimations();

    void bounceWithVelocity(NXPoint velocity);

    friend class UIScrollViewFocusTestHarness;
    friend class UIScrollViewIndicatorTestHarness;
};

}
