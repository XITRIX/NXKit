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

void UIScrollView::setContentOffset(Point offset) {
    if (getBounds().origin == offset) return;
    
    setBounds({ offset, getBounds().size });
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
        setContentOffset(offset);
        lastSafeAreaInset = safeAreaInset;
    }
}

void UIScrollView::onPan() {
    Point translation = panGestureRecognizer->translationInView(this);
    panGestureRecognizer->setTranslation(Point(), this);

    Point newOffset = getContentOffset() - translation;
    newOffset = getContentOffsetInBounds(newOffset);
    setContentOffset(newOffset);
}

Point UIScrollView::getContentOffsetInBounds(Point offset) {
    Size contenSize = getContentSize();

    if (contenSize.width <= getFrame().size.width - lastSafeAreaInset.left - lastSafeAreaInset.right) {
        offset.x = -lastSafeAreaInset.left;
    } else {
        if (offset.x < -lastSafeAreaInset.left)
            offset.x = -lastSafeAreaInset.left;
        if (offset.x > getFrame().size.width - contenSize.width - lastSafeAreaInset.right)
            offset.x = getFrame().size.width - contenSize.width - lastSafeAreaInset.right;
    }

    if (contenSize.height <= getFrame().size.height - lastSafeAreaInset.top - lastSafeAreaInset.bottom) {
        offset.y = -lastSafeAreaInset.top;
    } else {
        if (offset.y < -lastSafeAreaInset.top)
            offset.y = -lastSafeAreaInset.top;
        if (offset.y > contenSize.height - getFrame().size.height + lastSafeAreaInset.bottom)
            offset.y = contenSize.height - getFrame().size.height + lastSafeAreaInset.bottom;
    }

    return offset;
}

}
