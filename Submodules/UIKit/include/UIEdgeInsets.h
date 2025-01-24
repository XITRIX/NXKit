//
//  UIEdgeInsets.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 12.03.2023.
//

#pragma once

#include <Geometry.h>

namespace NXKit {

struct UIEdgeInsets {
    float top;
    float left;
    float bottom;
    float right;

    UIEdgeInsets(float top, float left, float bottom, float right);
    UIEdgeInsets(): UIEdgeInsets(0, 0, 0, 0) {}

    bool operator==(const UIEdgeInsets& rhs) const;
    bool operator!=(const UIEdgeInsets& rhs) const;
    UIEdgeInsets operator+(const UIEdgeInsets& rhs) const;
    UIEdgeInsets operator-(const UIEdgeInsets& rhs) const;
    UIEdgeInsets& operator+=(const UIEdgeInsets& rhs);
    UIEdgeInsets& operator-=(const UIEdgeInsets& rhs);

    static UIEdgeInsets zero;
};

}
