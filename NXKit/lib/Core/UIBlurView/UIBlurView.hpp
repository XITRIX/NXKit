//
//  UIBlurView.hpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

#include "UIStackView.hpp"

namespace NXKit {

class UIBlurView: public UIStackView {
public:
    UIBlurView(Rect frame);
    UIBlurView(float x, float y, float width, float height): UIBlurView(Rect(x, y, width, height)) {}
    UIBlurView(): UIBlurView(Rect(0, 0, UIView::AUTO, UIView::AUTO)) {}

    float blurRadius = 6;

    void draw(NVGcontext *vgContext) override;
};

}
