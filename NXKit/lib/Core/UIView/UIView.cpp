//
//  UIView.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include "UIView.hpp"

UIView::UIView(Rect frame):
    frame(frame),
    backgroundColor(0, 0, 0, 0)
{ }

void UIView::internalDraw(NVGcontext* vgContext) {
    // Background color
    nvgBeginPath(vgContext);
    nvgRoundedRect(vgContext, frame.origin.x, frame.origin.y, frame.size.width, frame.size.height, cornerRadius);
    nvgFillColor(vgContext, backgroundColor.raw());
    nvgFill(vgContext);

    draw(vgContext);

    nvgSave(vgContext);
    nvgTranslate(vgContext, frame.origin.x, frame.origin.y);
    for (auto view: subviews) {
        view->internalDraw(vgContext);
    }
    nvgRestore(vgContext);


    // Borders
    if (borderThickness > 0) {
        float offset = borderThickness / 2;
        Rect borderRect = Rect(frame.origin.x + offset, frame.origin.y + offset, frame.size.width - offset * 2, frame.size.height - offset * 2);

        nvgBeginPath(vgContext);
        nvgStrokeColor(vgContext, borderColor.raw());
        nvgStrokeWidth(vgContext, this->borderThickness);
        nvgRoundedRect(vgContext, borderRect.origin.x, borderRect.origin.y, borderRect.size.width, borderRect.size.height, this->cornerRadius);
        nvgStroke(vgContext);
    }
}

void UIView::addSubview(UIView *subview) {
    subview->superview = this;
    subviews.push_back(subview);
}

std::vector<UIView*> UIView::getSubviews() {
    return subviews;
}

UIView* UIView::getSuperview() {
    return superview;
}
