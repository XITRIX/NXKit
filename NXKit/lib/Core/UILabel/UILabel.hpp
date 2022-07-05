//
//  UILabel.hpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

#include "UIView.hpp"

class UILabel: public UIView {
    void draw(NVGcontext *vgContext) override;
};
