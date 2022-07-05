//
//  UIBlurView.hpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

#include "UIView.hpp"

class UIBlurView: public UIView {
public:
    UIBlurView(Rect frame);
    UIBlurView(): UIView(Rect()) {}

    float blurRadius = 24;

    void draw(NVGcontext *vgContext) override;
};
