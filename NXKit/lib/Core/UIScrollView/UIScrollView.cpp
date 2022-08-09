//
//  UIScrollView.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 17.07.2022.
//

#include "UIScrollView.hpp"

namespace NXKit {

NXPoint UIScrollView::getContentOffset() {
    return getBounds().origin;
}

void UIScrollView::setContentOffset(NXPoint offset) {
    setBounds({ offset, getBounds().size });
}

}
