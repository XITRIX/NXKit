//
//  UIEdgeInsets.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 19.08.2022.
//

#pragma once

namespace NXKit {

struct UIEdgeInsets {
    float top, left, bottom, right;

    UIEdgeInsets();
    UIEdgeInsets(float top, float left, float bottom, float right);
    bool operator==(const UIEdgeInsets& rhs);
    bool operator!=(const UIEdgeInsets& rhs);
    void operator+=(const UIEdgeInsets& rhs);

    static const UIEdgeInsets zero;
};

}
