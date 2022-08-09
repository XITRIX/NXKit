//
//  UIScrollView.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 17.07.2022.
//

#pragma once

#include <Core/UIView/UIView.hpp>

namespace NXKit {

class UIScrollView: public UIView {
public:
    NXPoint getContentOffset();
    void setContentOffset(NXPoint offset);
};

}
