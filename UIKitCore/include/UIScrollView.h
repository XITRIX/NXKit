//
//  UIScrollView.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 12.03.2023.
//

#pragma once

#include <UIView.h>
#include <UIPanGestureRecognizer.h>
#include <UIScrollViewExtensions/DecelerationTimingParameters.h>
#include <UIScrollViewExtensions/TimerAnimation.h>
#include <UIScrollViewExtensions/UIScrollViewDecelerationRate.h>

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

class UIScrollView: public UIView {
public:
    static std::shared_ptr<UIView> init() { return new_shared<UIScrollView>(); }

    std::weak_ptr<UIScrollViewDelegate> delegate;

    UIScrollView(NXRect frame = NXRect());

//    virtual void addSubview(std::shared_ptr<UIView> view) override;
//    bool applyXMLAttribute(std::string name, std::string value) override;
    void layoutSubviews() override;

    void layoutMarginsDidChange() override;

    NXPoint contentOffset() { return bounds().origin; }
    void setContentOffset(NXPoint offset, bool animated);

    UIEdgeInsets contentInset() { return _contentInset; }
    void setContentInset(UIEdgeInsets contentInset) { _contentInset = contentInset; }

    bool bounceHorizontally() const { return _bounceHorizontally; }
    void setBounceHorizontally(bool bounceHorizontally);

    bool bounceVertically() const { return _bounceVertically; }
    void setBounceVertically(bool bounceVertically);

    UIScrollViewDecelerationRate decelerationRate() const { return _decelerationRate; }
    void setDecelerationRate(UIScrollViewDecelerationRate decelerationRate) { _decelerationRate = decelerationRate; }

    UIScrollViewContentInsetAdjustmentBehavior contentInsetAdjustmentBehavior() { return _contentInsetAdjustmentBehavior; }
    void setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior contentInsetAdjustmentBehavior);

    NXSize contentSize();
//    void setContentSize(Size size) { _contentSize = size; }

private:
    std::shared_ptr<UIPanGestureRecognizer> _panGestureRecognizer;
    bool _isDecelerating = false;
    NXPoint weightedAverageVelocity;
    NXPoint _initialContentOffset;
    UIScrollViewDecelerationRate _decelerationRate = UIScrollViewDecelerationRate::normal;

    bool _bounceHorizontally = false;
    bool _bounceVertically = false;

    bool shouldBounceHorizontally();
    bool shouldBounceVertically();

    UIScrollViewContentInsetAdjustmentBehavior _contentInsetAdjustmentBehavior = UIScrollViewContentInsetAdjustmentBehavior::scrollableAxes;

    std::shared_ptr<TimerAnimation> _timerAnimation;
    UIEdgeInsets _lastLayoutMargins;
    UIEdgeInsets _contentInset;
//    Size _contentSize;

    void onPan();
    void onPanGestureStateChanged();

    NXPoint visibleContentOffset();
    NXPoint getBoundsCheckedContentOffset(NXPoint newContentOffset);

    NXRect contentOffsetBounds();

    void layoutScrollIndicatorsIfNeeded();
    void showScrollIndicators();
    void hideScrollIndicators();

    void startDeceleratingIfNecessary();
    void cancelDeceleratingIfNeccessary();
    void cancelDecelerationAnimations();

    void bounceWithVelocity(NXPoint velocity);
};

}
