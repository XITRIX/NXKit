//
//  UIEdgeInsets.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 12.03.2023.
//

#include <UIEdgeInsets.h>

namespace NXKit {

UIEdgeInsets::UIEdgeInsets(float top, float left, float bottom, float right):
        top(top), left(left), bottom(bottom), right(right)
{ }

bool UIEdgeInsets::operator==(const UIEdgeInsets& rhs) const {
    return top == rhs.top &&
           left == rhs.left &&
           bottom == rhs.bottom &&
           right == rhs.right;
}

bool UIEdgeInsets::operator!=(const UIEdgeInsets& rhs) const {
    return !(*this == rhs);
}

UIEdgeInsets UIEdgeInsets::operator+(const UIEdgeInsets& rhs) const {
    return {top + rhs.top, left + rhs.left, bottom + rhs.bottom, right + rhs.right};
}

UIEdgeInsets UIEdgeInsets::operator-(const UIEdgeInsets& rhs) const {
    return {top - rhs.top, left - rhs.left, bottom - rhs.bottom, right - rhs.right};
}

UIEdgeInsets& UIEdgeInsets::operator+=(const UIEdgeInsets& rhs) {
    this->top += rhs.top;
    this->left += rhs.left;
    this->bottom += rhs.bottom;
    this->right += rhs.right;
    return *this;
}

UIEdgeInsets& UIEdgeInsets::operator-=(const UIEdgeInsets& rhs) {
    this->top -= rhs.top;
    this->left -= rhs.left;
    this->bottom -= rhs.bottom;
    this->right -= rhs.right;
    return *this;
}

UIEdgeInsets UIEdgeInsets::zero = UIEdgeInsets();

}
