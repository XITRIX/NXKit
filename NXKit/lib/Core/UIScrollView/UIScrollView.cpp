//
//  UIScrollView.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 17.07.2022.
//

#include <Core/UIScrollView/UIScrollView.hpp>

namespace NXKit {

#define PAN_SCROLL_ACCELERATION -50
#define SCROLLING_INDICATOR_WIDTH 4

UIScrollView::UIScrollView() {
    scrollingIndicatorV = new UIView();
    scrollingIndicatorV->setWidth(SCROLLING_INDICATOR_WIDTH);
    scrollingIndicatorV->cornerRadius = SCROLLING_INDICATOR_WIDTH / 2;
    scrollingIndicatorV->backgroundColor = UIColor::gray;
    insertSubview(scrollingIndicatorV, 0);

    panGestureRecognizer->onStateChanged = [this](UIGestureRecognizerState state) {
        switch (state) {
            case UIGestureRecognizerState::POSSIBLE:
                cancelDeceleting();
                break;
            case UIGestureRecognizerState::CHANGED:
                if (!dragging) {
                    dragging = true;
                    if (delegate) delegate->scrollViewWillBeginDragging(this);
                }
                onPan();
                break;
            case UIGestureRecognizerState::ENDED:
                dragging = false;
                startDeceleting();
                break;
            default: break;
        }
    };
    addGestureRecognizer(panGestureRecognizer);
}

void UIScrollView::addSubview(UIView *view) {
    if (contentView != nullptr) return;

    contentView = view;
    insertSubview(view, 0);
}

void UIScrollView::startDeceleting() {
    Point velocity = panGestureRecognizer->velocityIn(this);
    if (velocity.magnitude() <= 0) return;

    decelerating = true;

    float time = -fabs(velocity.magnitude()) / PAN_SCROLL_ACCELERATION;
    Point distance = Point(velocity.x * time / 2, velocity.y * time / 2);

    animate(time / 10, [this, distance]() {
        auto offset = getContentOffset() - distance;
        setBounds({offset , getBounds().size });
    }, EasingFunction::quadraticOut, [this](bool res) {
        decelerating = false;
    });

    printf("Velocity: X-%f, Y-%f\n", velocity.x, velocity.y);
}

void UIScrollView::cancelDeceleting() {
    if (!decelerating) return;
    animate(0, [](){}, EasingFunction::linear, [this](bool res){
        decelerating = false;
    });
}

Point UIScrollView::getContentOffset() {
    return getBounds().origin;
}

void UIScrollView::setContentOffset(Point offset, bool animated) {
    if (!offset.valid()) return;

    if (getBounds().origin == offset) return;

    if (animated) {
        animate(0.1f, [this, offset]() {
//            setContentOffset(offset, false);
            setBounds({ offset, getBounds().size });
        });
    } else {
        setBounds({ offset, getBounds().size });
    }
    updateScrollingIndicatior();
}

void UIScrollView::updateScrollingIndicatior() {
    Rect visible = getVisibleBounds();
    scrollingIndicatorV->setHidden(visible.height() > getContentSize().height);
    float height = visible.height() * (visible.height() / getContentSize().height);
    scrollingIndicatorV->setHeight(height);

    float pos = (getContentOffset().y + visible.minY()) * (visible.height() / getContentSize().height) + visible.minY();
    scrollingIndicatorV->setPosition(Point(getFrame().width() - 16, pos + getContentOffset().y));
}

void UIScrollView::setBounds(Rect bounds) {
    if (!bounds.origin.valid()) return;
    UIView::setBounds(bounds);
    if (delegate) delegate->scrollViewDidScroll(this);
}

std::deque<float> UIScrollView::createAnimationContext() {
    std::deque<float> context = UIView::createAnimationContext();

    context.push_back(getContentOffset().x);
    context.push_back(getContentOffset().y);

    return context;
}

void UIScrollView::applyAnimationContext(std::deque<float>* context) {
    UIView::applyAnimationContext(context);

    auto newOffset = Point(pop(context), pop(context));
    newOffset = getContentOffsetInBounds(newOffset);
    setContentOffset(newOffset, false);

    updateScrollingIndicatior();
}

void UIScrollView::setFixWidth(bool fix) {
    fixWidth = true;
    setNeedsLayout();
}

void UIScrollView::setFixHeight(bool fix) {
    fixHeight = true;
    setNeedsLayout();
}

Size UIScrollView::getContentSize() {
    if (!contentView) return Size();
    return contentView->getFrame().size;
}

void UIScrollView::subviewFocusDidChange(UIView *focusedView, UIView *notifiedView) {
    if (!getFrame().valid()) return UIView::subviewFocusDidChange(focusedView, notifiedView);

    if (scrollingMode == UIScrollViewScrollingMode::centered) {
        Rect focusedViewFrame = focusedView->getFrame();
        Point origin = focusedView->getSuperview()->convert(focusedViewFrame.origin, this);
        origin.y -= getFrame().height() / 2 - focusedViewFrame.height() / 2;
        Point newOffset = getContentOffsetInBounds(origin);
        setContentOffset(newOffset);
    }

    if (scrollingMode == UIScrollViewScrollingMode::scrollingEdge) {
        Rect bounds = getVisibleOffsetBounds();
        Point newOffset = bounds.origin;
        Rect focusedViewFrame = focusedView->getFrame();
        focusedViewFrame.origin = focusedView->getSuperview()->convert(focusedViewFrame.origin, this);

        float padding = 20;

        if (focusedViewFrame.minY() < bounds.minY()) newOffset.y = focusedViewFrame.minY() - safeAreaInsets().top - padding;
        if (focusedViewFrame.maxY() > bounds.maxY()) newOffset.y = focusedViewFrame.maxY() - safeAreaInsets().top + padding - bounds.height();

        if (focusedViewFrame.minX() < bounds.minX()) newOffset.x = focusedViewFrame.minX() - safeAreaInsets().left - padding;
        if (focusedViewFrame.maxX() > bounds.maxX()) newOffset.x = focusedViewFrame.maxX() - safeAreaInsets().left + padding - bounds.width();

        if (newOffset == bounds.origin) return UIView::subviewFocusDidChange(focusedView, notifiedView);

        newOffset = getContentOffsetInBounds(newOffset);
        setContentOffset(newOffset);
    }

    UIView::subviewFocusDidChange(focusedView, notifiedView);
}

void UIScrollView::layoutSubviews() {
    UIView::layoutSubviews();

    if (contentView) {
        Size frameSize = getBounds().size;
        Size size = Size(fixWidth ? frameSize.width : UIView::AUTO, fixHeight ? frameSize.height : UIView::AUTO);
        contentView->setSize(size);
    }

    auto safeAreaInset = safeAreaInsets();
    if (lastSafeAreaInset != safeAreaInset) {
        auto offset = getContentOffset();
        Point latestSafeAreaOffset = Point(lastSafeAreaInset.left, lastSafeAreaInset.top);
        Point safeAreaOffset = Point(safeAreaInset.left, safeAreaInset.top);
        offset += latestSafeAreaOffset - safeAreaOffset;
        setContentOffset(offset, false);
        lastSafeAreaInset = safeAreaInset;
    }

    updateScrollingIndicatior();
}

UIView* UIScrollView::getNextFocus(NavigationDirection direction) {
    Rect offsetBounds = getContentOffsetBounds();

    if (direction == NavigationDirection::UP && getContentOffset().y > offsetBounds.minY()) {
        setContentOffset(Point(getContentOffset().x, offsetBounds.minY()));
        return nullptr;
    }
    if (direction == NavigationDirection::LEFT && getContentOffset().x > offsetBounds.minX()) {
        setContentOffset(Point(offsetBounds.minX(), getContentOffset().y));
        return nullptr;
    }
    if (direction == NavigationDirection::DOWN && getContentOffset().y < offsetBounds.maxY()) {
        setContentOffset(Point(getContentOffset().x, offsetBounds.maxY()));
        return nullptr;
    }
    if (direction == NavigationDirection::LEFT && getContentOffset().x > offsetBounds.maxX()) {
        setContentOffset(Point(offsetBounds.maxX(), getContentOffset().y));
        return nullptr;
    }

    return UIView::getNextFocus(direction);
}

void UIScrollView::onPan() {
    Point translation = panGestureRecognizer->translationInView(this);
    panGestureRecognizer->setTranslation(Point(), this);

    Point newOffset = getContentOffset() - translation;
    newOffset = getContentOffsetInBounds(newOffset);
    setContentOffset(newOffset, false);
}

Rect UIScrollView::getContentOffsetBounds() {
    UIEdgeInsets safeArea = safeAreaInsets();
    Point origin = Point(-safeArea.left, -safeArea.top);
    Size size = getContentSize();
    size.width += safeArea.left + safeArea.right - getFrame().size.width;
    size.height += safeArea.top + safeArea.bottom - getFrame().size.height;
    return Rect(origin, size);
}

Rect UIScrollView::getVisibleBounds() {
    Point origin = Point(lastSafeAreaInset.left, lastSafeAreaInset.top);
    Size size = getBounds().size;
    size.width -= lastSafeAreaInset.left + lastSafeAreaInset.right;
    size.height -= lastSafeAreaInset.top + lastSafeAreaInset.bottom;
    return Rect(origin, size);
}

Rect UIScrollView::getVisibleOffsetBounds() {
    auto bounds = getBounds();
    bounds.origin.y += safeAreaInsets().top;
    bounds.origin.x += safeAreaInsets().left;
    bounds.size.height -= safeAreaInsets().top + safeAreaInsets().bottom;
    bounds.size.width -= safeAreaInsets().left + safeAreaInsets().right;
    return bounds;
}

Point UIScrollView::getContentOffsetInBounds(Point offset) {
    Size contentSize = getContentSize();
    Rect offsetBounds = getContentOffsetBounds();
    Rect visibleBounds = getVisibleBounds();

    if (offset.x < offsetBounds.minX() || contentSize.width < visibleBounds.width()) offset.x = offsetBounds.minX();
    else if (offset.x > offsetBounds.maxX()) offset.x = offsetBounds.maxX();
    if (offset.y < offsetBounds.minY() || contentSize.height < visibleBounds.height()) offset.y = offsetBounds.minY();
    else if (offset.y > offsetBounds.maxY()) offset.y = offsetBounds.maxY();

    return offset;
}

}
