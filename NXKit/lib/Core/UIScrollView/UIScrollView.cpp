//
//  UIScrollView.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 17.07.2022.
//

#include "UIScrollView.hpp"

namespace NXKit {

UIScrollView::UIScrollView() {
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

void UIScrollView::layoutSubviews() {
    UIView::layoutSubviews();

    if (getSubviews().size() > 0) {
        Size frameSize = frame.size();
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

}
