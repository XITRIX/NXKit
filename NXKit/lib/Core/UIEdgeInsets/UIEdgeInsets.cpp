//
//  UIEdgeInsets.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 19.08.2022.
//

#include "UIEdgeInsets.hpp"

namespace NXKit {

UIEdgeInsets::UIEdgeInsets():
    top(0), left(0), bottom(0), right(0)
{}

UIEdgeInsets::UIEdgeInsets(float top, float left, float bottom, float right):
    top(top), left(left), bottom(bottom), right(right)
{}

bool UIEdgeInsets::operator==(const UIEdgeInsets& rhs) {
    return top == rhs.top && left == rhs.left && bottom == rhs.bottom && right == rhs.right;
}

bool UIEdgeInsets::operator!=(const UIEdgeInsets& rhs) {
    return !(*this == rhs);
}

void UIEdgeInsets::operator+=(const UIEdgeInsets& rhs) {
    top += rhs.top;
    left += rhs.left;
    bottom += rhs.bottom;
    right += rhs.right;
}

const UIEdgeInsets UIEdgeInsets::zero = UIEdgeInsets(0, 0, 0, 0);

}
