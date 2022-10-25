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
    UIEdgeInsets contentInset;
    std::shared_ptr<UIScrollViewDelegate> delegate;
    UIScrollViewScrollingMode scrollingMode = UIScrollViewScrollingMode::centered;

    UIScrollView();
    Point getContentOffset();
    virtual void setContentOffset(Point offset, bool animated = true);

    void setBounds(Rect bounds) override;
    void addSubview(std::shared_ptr<UIView> view) override;
    std::deque<float> createAnimationContext() override;
    void applyAnimationContext(std::deque<float>* context) override;

    void subviewFocusDidChange(std::shared_ptr<UIView> focusedView, std::shared_ptr<UIView> notifiedView) override;
    void layoutSubviews() override;
    std::shared_ptr<UIView> getNextFocus(NavigationDirection direction) override;

    void setFixWidth(bool fix);
    void setFixHeight(bool fix);

    virtual Size getContentSize();

private:
    std::shared_ptr<UIPanGestureRecognizer> panGestureRecognizer;
    UIEdgeInsets lastSafeAreaInset;
    bool fixWidth = false, fixHeight = false;
    bool dragging = false;
    bool decelerating = false;

    std::shared_ptr<UIView> contentView = nullptr;

    std::shared_ptr<UIView> scrollingIndicatorV = nullptr;

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
