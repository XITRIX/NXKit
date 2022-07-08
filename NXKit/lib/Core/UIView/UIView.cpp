//
//  UIView.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include "UIView.hpp"

UIView::UIView(Rect frame):
    backgroundColor(0, 0, 0, 0)
{
    this->ygNode = YGNodeNew();
    YGNodeSetContext(this->ygNode, this);

    setPosition(frame.origin);
    setSize(frame.size);
}

void UIView::setPosition(Point position) {
    YGNodeStyleSetPosition(this->ygNode, YGEdgeLeft, position.x);
    YGNodeStyleSetPosition(this->ygNode, YGEdgeTop, position.y);
    setNeedsLayout();
}

void UIView::setSize(Size size) {
    if (size.width == UIView::AUTO)
    {
        YGNodeStyleSetWidthAuto(this->ygNode);
        YGNodeStyleSetMinWidth(this->ygNode, YGUndefined);
    }
    else
    {
        YGNodeStyleSetWidth(this->ygNode, size.width);
        YGNodeStyleSetMinWidth(this->ygNode, size.width);
    }

    if (size.height == UIView::AUTO)
    {
        YGNodeStyleSetHeightAuto(this->ygNode);
        YGNodeStyleSetMinHeight(this->ygNode, YGUndefined);
    }
    else
    {
        YGNodeStyleSetHeight(this->ygNode, size.height);
        YGNodeStyleSetMinHeight(this->ygNode, size.height);
    }
    setNeedsLayout();
}

Rect UIView::getFrame() {
    return Rect( YGNodeLayoutGetLeft(this->ygNode), YGNodeLayoutGetTop(this->ygNode), YGNodeLayoutGetWidth(this->ygNode), YGNodeLayoutGetHeight(this->ygNode) );
}

Rect UIView::getBounds() {
    Rect frame = getFrame();
    frame.origin = Point();
    return frame;
}

void UIView::internalDraw(NVGcontext* vgContext) {
    layoutIfNeeded();

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

    if (getBorderTop() > 0) {
        nvgBeginPath(vgContext);
        nvgRect(vgContext, frame.origin.x, frame.origin.y, frame.size.width, getBorderTop());
        nvgFillColor(vgContext, borderColor.raw());
        nvgFill(vgContext);
    }

    if (getBorderLeft() > 0) {
        nvgBeginPath(vgContext);
        nvgRect(vgContext, frame.origin.x, frame.origin.y, getBorderLeft(), frame.size.height);
        nvgFillColor(vgContext, borderColor.raw());
        nvgFill(vgContext);
    }

    if (getBorderRight() > 0) {
        nvgBeginPath(vgContext);
        nvgRect(vgContext, (frame.origin.x + frame.size.width - getBorderRight()), frame.origin.y, getBorderRight(), frame.size.height);
        nvgFillColor(vgContext, borderColor.raw());
        nvgFill(vgContext);
    }

    if (getBorderBottom() > 0) {
        nvgBeginPath(vgContext);
        nvgRect(vgContext, frame.origin.x, (frame.origin.y + frame.size.height - getBorderBottom()), frame.size.width, getBorderBottom());
        nvgFillColor(vgContext, borderColor.raw());
        nvgFill(vgContext);
    }
}

void UIView::setNeedsLayout() {
    YGNode* parentNode = YGNodeGetParent(this->ygNode);
    if (!parentNode) {
        needsLayout = true;
        return;
    }

    UIView* nodeHolder = (UIView*) YGNodeGetParent(this->ygNode)->getContext();
    if (!nodeHolder) {
        needsLayout = true;
        return;
    }

    nodeHolder->setNeedsLayout();
}

void UIView::layoutIfNeeded() {
    YGNode* parentNode = YGNodeGetParent(this->ygNode);
    if (!parentNode) {
        if (needsLayout)
            layoutSubviews();
        return;
    }

    UIView* nodeHolder = (UIView*) YGNodeGetParent(this->ygNode)->getContext();
    if (!nodeHolder) {
        if (needsLayout)
            layoutSubviews();
        return;
    }

    if (nodeHolder->needsLayout)
        nodeHolder->layoutSubviews();
}

void UIView::layoutSubviews() {
    needsLayout = false;

    if (YGNodeHasMeasureFunc(this->ygNode))
        YGNodeMarkDirty(this->ygNode);

    if (!YGNodeGetParent(this->ygNode)) {
        YGNodeCalculateLayout(this->ygNode, YGUndefined, YGUndefined, YGDirectionLTR);
    }

    for (auto view: getSubviews()) {
        view->layoutSubviews();
    }

    frame = getFrame();

//    YGNode* parentNode = YGNodeGetParent(this->ygNode);
//    if (!parentNode) {
//        YGNodeCalculateLayout(this->ygNode, YGUndefined, YGUndefined, YGDirectionLTR);
//        return;
//    }
//
//    UIView* nodeHolder = (UIView*) YGNodeGetParent(this->ygNode)->getContext();
//    if (!nodeHolder) {
//        YGNodeCalculateLayout(this->ygNode, YGUndefined, YGUndefined, YGDirectionLTR);
//        return;
//    }
//
//    nodeHolder->layoutSubviews();
}

void UIView::addSubview(UIView *view) {
    view->superview = this;
    subviews.push_back(view);
    layoutSubviews();
}

std::vector<UIView*> UIView::getSubviews() {
    return subviews;
}

Point UIView::convert(Point point, UIView* toView) {
    Point selfAbsoluteOrigin;
    Point otherAbsoluteOrigin;

    UIView* current = this;
    while (current) {
        selfAbsoluteOrigin += current->frame.origin;
        if (current == toView) {
            return point + selfAbsoluteOrigin;
        }
        current = current->superview;
    }

    current = toView;
    while (current) {
        otherAbsoluteOrigin += current->frame.origin;
        current = current->superview;
    }

    Point originDifference = otherAbsoluteOrigin - selfAbsoluteOrigin;
    return point - originDifference;
}

UIView* UIView::getSuperview() {
    return superview;
}

void UIView::setGrow(float grow) {
    YGNodeStyleSetFlexGrow(this->ygNode, grow);
}

float UIView::getGrow() {
    return YGNodeStyleGetFlexGrow(this->ygNode);
}

void UIView::setMargins(float top, float right, float bottom, float left) {
    setMarginTop(top);
    setMarginRight(right);
    setMarginBottom(bottom);
    setMarginLeft(left);
}

void UIView::setMargins(float margins) {
    setMargins(margins, margins, margins, margins);
}

void UIView::setMarginTop(float top) {
    if (top == UIView::AUTO)
        YGNodeStyleSetMarginAuto(this->ygNode, YGEdgeTop);
    else
        YGNodeStyleSetMargin(this->ygNode, YGEdgeTop, top);

    this->setNeedsLayout();
}

void UIView::setMarginRight(float right) {
    if (right == UIView::AUTO)
        YGNodeStyleSetMarginAuto(this->ygNode, YGEdgeRight);
    else
        YGNodeStyleSetMargin(this->ygNode, YGEdgeRight, right);

    this->setNeedsLayout();
}

void UIView::setMarginBottom(float bottom) {
    if (bottom == UIView::AUTO)
        YGNodeStyleSetMarginAuto(this->ygNode, YGEdgeBottom);
    else
        YGNodeStyleSetMargin(this->ygNode, YGEdgeBottom, bottom);

    this->setNeedsLayout();
}

void UIView::setMarginLeft(float left) {
    if (left == UIView::AUTO)
        YGNodeStyleSetMarginAuto(this->ygNode, YGEdgeLeft);
    else
        YGNodeStyleSetMargin(this->ygNode, YGEdgeLeft, left);

    this->setNeedsLayout();
}

void UIView::setBorderTop(float top) {
    YGNodeStyleSetBorder(this->ygNode, YGEdgeTop, top);
}

void UIView::setBorderLeft(float left) {
    YGNodeStyleSetBorder(this->ygNode, YGEdgeLeft, left);
}

void UIView::setBorderRight(float right) {
    YGNodeStyleSetBorder(this->ygNode, YGEdgeRight, right);
}

void UIView::setBorderBottom(float bottom) {
    YGNodeStyleSetBorder(this->ygNode, YGEdgeBottom, bottom);
}

float UIView::getBorderTop() {
    return YGNodeLayoutGetBorder(this->ygNode, YGEdgeTop);
}

float UIView::getBorderLeft() {
    return YGNodeLayoutGetBorder(this->ygNode, YGEdgeLeft);
}

float UIView::getBorderRight() {
    return YGNodeLayoutGetBorder(this->ygNode, YGEdgeRight);
}

float UIView::getBorderBottom() {
    return YGNodeLayoutGetBorder(this->ygNode, YGEdgeBottom);
}
