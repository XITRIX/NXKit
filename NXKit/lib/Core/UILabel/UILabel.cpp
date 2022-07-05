//
//  UILabel.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include "UILabel.hpp"

void UILabel::draw(NVGcontext *vgContext) {
//    nvgBeginPath(vgContext);
//    nvgRoundedRect(vgContext, frame.origin.x, frame.origin.y, frame.size.width, frame.size.height, cornerRadius);
//    nvgFillColor(vgContext, UIColor(155, 155, 155).raw());
//    nvgFill(vgContext);

    nvgBeginPath(vgContext);
    nvgFontFace(vgContext, "sans");
    nvgFontSize(vgContext, 38.0f);
    nvgTextAlign(vgContext, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
    nvgFillColor(vgContext, nvgRGBA(0, 0, 0, 255));
    nvgTextBox(vgContext, frame.origin.x, frame.origin.y, frame.size.width, "Test text", nullptr);
}
