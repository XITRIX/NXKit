//
//  UIScrollView.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 17.07.2022.
//

#pragma once

#include <Core/UIView/UIView.hpp>
#include <Core/UIPanGestureRecognizer/UIPanGestureRecognizer.hpp>

namespace NXKit {

enum class UIScrollViewScrollingMode {
    centered,
    scrollingEdge,
    continous
};

class UIScrollView;
class UIScrollViewDelegate {
public:
    virtual void scrollViewWillBeginDragging(UIScrollView* scrollView) {}
    virtual void scrollViewDidScroll(UIScrollView* scrollView) {}
    virtual void scrollViewDidEndDragging(UIScrollView* scrollView) {}
};

class UIScrollView: public UIView {
public:
    UIScrollViewDelegate* delegate = nullptr;
    UIScrollViewScrollingMode scrollingMode = UIScrollViewScrollingMode::centered;

    UIScrollView();
    Point getContentOffset();
    void setContentOffset(Point offset, bool animated = true);

    void setBounds(Rect bounds) override;
    void addSubview(UIView *view) override;
    std::deque<float> createAnimationContext() override;
    void applyAnimationContext(std::deque<float>* context) override;

    void subviewFocusDidChange(UIView *focusedView, UIView *notifiedView) override;
    void layoutSubviews() override;
    UIView* getNextFocus(NavigationDirection direction) override;

    void setFixWidth(bool fix);
    void setFixHeight(bool fix);

    Size getContentSize();

private:
    UIPanGestureRecognizer* panGestureRecognizer = new UIPanGestureRecognizer();
    UIEdgeInsets lastSafeAreaInset;
    bool fixWidth = false, fixHeight = false;
    bool dragging = false;
    bool decelerating = false;

    UIView *contentView = nullptr;

    UIView* scrollingIndicatorV = nullptr;

    Rect getContentOffsetBounds();
    Rect getVisibleBounds();
    Point getContentOffsetInBounds(Point offset);
    Rect getVisibleOffsetBounds();

    void startDeceleting();
    void cancelDeceleting();
    void updateScrollingIndicatior();

    void onPan();
};

}
