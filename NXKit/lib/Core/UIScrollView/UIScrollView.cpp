//
//  UIScrollView.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 17.07.2022.
//

#include <Core/UIScrollView/UIScrollView.hpp>

namespace NXKit {

UIScrollView::UIScrollView() {
//    clipToBounds = true;
    panGestureRecognizer->onStateChanged = [this](UIGestureRecognizerState state) {
        switch (state) {
            case UIGestureRecognizerState::CHANGED:
                if (!dragging) {
                    dragging = true;
                    if (delegate) delegate->scrollViewWillBeginDragging(this);
                }
                onPan();
                break;
            case UIGestureRecognizerState::ENDED:
                dragging = false;
            default: break;
        }
    };
    addGestureRecognizer(panGestureRecognizer);
}

Point UIScrollView::getContentOffset() {
    return getBounds().origin;
}

void UIScrollView::setContentOffset(Point offset, bool animated) {
    if (getBounds().origin == offset) return;

    animate(animated ? 80 : 0, [this, offset]() {
        setBounds({ offset, getBounds().size });
    });
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
    if (getSubviews().size() == 0) return Size();
    return getSubviews()[0]->getFrame().size;
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
        Rect bounds = visibleBounds();
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

Rect UIScrollView::visibleBounds() {
    auto bounds = getBounds();
    bounds.origin.y += safeAreaInsets().top;
    bounds.origin.x += safeAreaInsets().left;
    bounds.size.height -= safeAreaInsets().top + safeAreaInsets().bottom;
    bounds.size.width -= safeAreaInsets().left + safeAreaInsets().right;
    return bounds;
}

void UIScrollView::layoutSubviews() {
    UIView::layoutSubviews();

    if (getSubviews().size() > 0) {
        Size frameSize = getBounds().size;
        Size size = Size(fixWidth ? frameSize.width : UIView::AUTO, fixHeight ? frameSize.height : UIView::AUTO);
        getSubviews()[0]->setSize(size);
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
    Point origin = Point(-lastSafeAreaInset.left, -lastSafeAreaInset.top);
    Size size = getContentSize();
    size.width += lastSafeAreaInset.left + lastSafeAreaInset.right - getFrame().size.width;
    size.height += lastSafeAreaInset.top + lastSafeAreaInset.bottom - getFrame().size.height;
    return Rect(origin, size);
}

Rect UIScrollView::getVisibleBounds() {
    Point origin = Point(lastSafeAreaInset.left, lastSafeAreaInset.top);
    Size size = getBounds().size;
    size.width -= lastSafeAreaInset.left + lastSafeAreaInset.right;
    size.height -= lastSafeAreaInset.top + lastSafeAreaInset.bottom;
    return Rect(origin, size);
}

Point UIScrollView::getContentOffsetInBounds(Point offset) {
    Rect offsetBounds = getContentOffsetBounds();
    Rect visibleBounds = getVisibleBounds();

    if (offset.x < offsetBounds.minX() || getContentSize().width < visibleBounds.width()) offset.x = offsetBounds.minX();
    else if (offset.x > offsetBounds.maxX()) offset.x = offsetBounds.maxX();
    if (offset.y < offsetBounds.minY() || getContentSize().height < visibleBounds.height()) offset.y = offsetBounds.minY();
    else if (offset.y > offsetBounds.maxY()) offset.y = offsetBounds.maxY();

    return offset;
}

}
