//
//  UIView.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include "UIView.hpp"
#include "UIViewController.hpp"
#include "Application.hpp"
#include "InputManager.hpp"

namespace NXKit {

UIView::UIView(Rect frame):
backgroundColor(0, 0, 0, 0)
{
    this->ygNode = YGNodeNew();
    YGNodeSetContext(this->ygNode, this);

    setPosition(frame.origin);
    setSize(frame.size);

    auto inputManager = InputManager::shared();
    inputManager->getInputUpdated()->subscribe([inputManager](){
        //        inputManager->
    });
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
    else if (0 < size.width && size.width < 1.0f)
    {
        YGNodeStyleSetMinWidthPercent(this->ygNode, size.width);
        YGNodeStyleSetWidthPercent(this->ygNode, size.width);
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
    else if (0 < size.height && size.height < 1.0f)
    {
        YGNodeStyleSetMinHeightPercent(this->ygNode, size.height);
        YGNodeStyleSetHeightPercent(this->ygNode, size.height);
    }
    else
    {
        YGNodeStyleSetHeight(this->ygNode, size.height);
        YGNodeStyleSetMinHeight(this->ygNode, size.height);
    }
    setNeedsLayout();
}

void UIView::setWidth(float width) {
    if (width == UIView::AUTO)
    {
        YGNodeStyleSetWidthAuto(this->ygNode);
        YGNodeStyleSetMinWidth(this->ygNode, YGUndefined);
    }
    else
    {
        YGNodeStyleSetWidth(this->ygNode, width);
        YGNodeStyleSetMinWidth(this->ygNode, width);
    }
    setNeedsLayout();
}

void UIView::setHeight(float height) {
    if (height == UIView::AUTO)
    {
        YGNodeStyleSetHeightAuto(this->ygNode);
        YGNodeStyleSetMinHeight(this->ygNode, YGUndefined);
    }
    else
    {
        YGNodeStyleSetHeight(this->ygNode, height);
        YGNodeStyleSetMinHeight(this->ygNode, height);
    }
    setNeedsLayout();
}

void UIView::setPercentWidth(float width) {
    YGNodeStyleSetMinWidthPercent(this->ygNode, width);
    YGNodeStyleSetWidthPercent(this->ygNode, width);
    setNeedsLayout();
}

void UIView::setPercentHeight(float height) {
    YGNodeStyleSetMinHeightPercent(this->ygNode, height);
    YGNodeStyleSetHeightPercent(this->ygNode, height);
    setNeedsLayout();
}

Rect UIView::getFrame() {
    return Rect( YGNodeLayoutGetLeft(this->ygNode), YGNodeLayoutGetTop(this->ygNode), YGNodeLayoutGetWidth(this->ygNode), YGNodeLayoutGetHeight(this->ygNode) );
}

Rect UIView::getBounds() {
    Rect frame = getFrame();
    frame.origin = bounds.origin;
    return frame;
}

void UIView::setBounds(Rect bounds) {
    this->bounds = bounds;
}

void UIView::internalDraw(NVGcontext* vgContext) {
    if (isHidden()) return;

    nvgSave(vgContext);
    layoutIfNeeded();

    // Frame position
    nvgTranslate(vgContext, frame.origin().x , frame.origin().y);

    // Position transform
    nvgTranslate(vgContext, transformOrigin.x() , transformOrigin.y());

    // Scale transform
    float scaleTransformX = (frame.size().width / 2.0f) - (frame.size().width / 2.0f) * transformSize.width();
    float scaleTransformY = (frame.size().height / 2.0f) - (frame.size().height / 2.0f) * transformSize.height();
    nvgTranslate(vgContext, scaleTransformX, scaleTransformY);
    nvgScale(vgContext, transformSize.width(), transformSize.height());

    // ClipToBounds
    if (clipToBounds)
        nvgIntersectScissor(vgContext, 0, 0, frame.size().width, frame.size().height);

    // Background color
    nvgBeginPath(vgContext);
    nvgRoundedRect(vgContext, 0, 0, frame.size().width, frame.size().height, cornerRadius);
    nvgFillColor(vgContext, backgroundColor.raw());
    nvgFill(vgContext);

    nvgSave(vgContext);
    nvgTranslate(vgContext, -bounds.minX(), -bounds.minY());
    draw(vgContext);

    nvgSave(vgContext);
    //    nvgTranslate(vgContext, frame.origin().x, frame.origin().y);
    for (auto view: subviews) {
        view->internalDraw(vgContext);
    }
    nvgRestore(vgContext);
    nvgRestore(vgContext);

    // Borders
    if (borderThickness > 0) {
        float offset = borderThickness / 2;
        Rect borderRect = Rect(offset, offset, frame.size().width - offset * 2, frame.size().height - offset * 2);

        nvgBeginPath(vgContext);
        nvgStrokeColor(vgContext, borderColor.raw());
        nvgStrokeWidth(vgContext, this->borderThickness);
        nvgRoundedRect(vgContext, borderRect.origin.x, borderRect.origin.y, borderRect.size.width, borderRect.size.height, this->cornerRadius);
        nvgStroke(vgContext);
    }

    if (getBorderTop() > 0) {
        nvgBeginPath(vgContext);
        nvgRect(vgContext, 0, 0, frame.size().width, getBorderTop());
        nvgFillColor(vgContext, borderColor.raw());
        nvgFill(vgContext);
    }

    if (getBorderLeft() > 0) {
        nvgBeginPath(vgContext);
        nvgRect(vgContext, 0, 0, getBorderLeft(), frame.size().height);
        nvgFillColor(vgContext, borderColor.raw());
        nvgFill(vgContext);
    }

    if (getBorderRight() > 0) {
        nvgBeginPath(vgContext);
        nvgRect(vgContext, (0 + frame.size().width - getBorderRight()), 0, getBorderRight(), frame.size().height);
        nvgFillColor(vgContext, borderColor.raw());
        nvgFill(vgContext);
    }

    if (getBorderBottom() > 0) {
        nvgBeginPath(vgContext);
        nvgRect(vgContext, 0, (frame.size().height - getBorderBottom()), frame.size().width, getBorderBottom());
        nvgFillColor(vgContext, borderColor.raw());
        nvgFill(vgContext);
    }

    nvgRestore(vgContext);
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
    if (controller) controller->viewWillLayoutSubviews();

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
    bounds = getBounds();

    if (controller) controller->viewDidLayoutSubviews();
}

bool UIView::isFocused() {
    return Application::shared()->getFocus() == this;
}

UIView* UIView::getDefaultFocus() {
    if (canBecomeFocused) return this;

    for (auto view: subviews) {
        UIView* focus = view->getDefaultFocus();
        if (focus) return focus;
    }

    return nullptr;
}

void UIView::subviewFocusDidChange(UIView* focusedView, UIView* notifiedView) {
    if (superview)
        superview->subviewFocusDidChange(focusedView, this);
}

UIView* UIView::getNextFocus(NavigationDirection direction) {
    if (!superview) return nullptr;
    return superview->getNextFocus(direction);
}

void UIView::addGestureRecognizer(UIGestureRecognizer* gestureRecognizer) {
    gestureRecognizers.push_back(gestureRecognizer);
}

std::vector<UIGestureRecognizer*> UIView::getGestureRecognizers() {
    return gestureRecognizers;
}

void UIView::addSubview(UIView *view) {
    view->setSuperview(this);
    subviews.push_back(view);
    setNeedsLayout();
}

UIResponder* UIView::getNext() {
    if (controller) return controller;
    if (superview) return superview;
    return nullptr;
}


void UIView::setHidden(bool hidden) {
    if (hidden)
        YGNodeStyleSetDisplay(this->ygNode, YGDisplayNone);
    else
        YGNodeStyleSetDisplay(this->ygNode, YGDisplayFlex);
    setNeedsLayout();
}

bool UIView::isHidden() {
    return YGNodeStyleGetDisplay(this->ygNode) == YGDisplayNone;
}

std::vector<UIView*> UIView::getSubviews() {
    return subviews;
}

Point UIView::convert(Point point, UIView* toView) {
    Point selfAbsoluteOrigin;
    Point otherAbsoluteOrigin;

    UIView* current = this;
    while (current) {
        if (current == toView) {
            return point - selfAbsoluteOrigin;
        }
        selfAbsoluteOrigin += current->frame.origin();
        selfAbsoluteOrigin -= current->bounds.origin;
        current = current->superview;
    }

    current = toView;
    while (current) {
        otherAbsoluteOrigin += current->frame.origin();
        otherAbsoluteOrigin -= current->bounds.origin;
        current = current->superview;
    }

    Point originDifference = otherAbsoluteOrigin - selfAbsoluteOrigin;
    return point - originDifference;
}

UIView* UIView::hitTest(Point point, UIEvent* withEvent) {
    if (!this->point(point, withEvent)) return nullptr;

    auto subviews = getSubviews();
    for (int i = (int) subviews.size()-1; i >= 0; i--) {
        Point convertedPoint = subviews[i]->convert(point, this);
        UIView* test = subviews[i]->hitTest(convertedPoint, withEvent);
        if (test) return test;
    }

    return this;
}

bool UIView::point(Point insidePoint, UIEvent *withEvent) {
    return bounds.contains(insidePoint);
}

UIView* UIView::getSuperview() {
    return superview;
}

void UIView::setSuperview(UIView* view) {
    superview = view;
}

void UIView::setGrow(float grow) {
    YGNodeStyleSetFlexGrow(this->ygNode, grow);
}

float UIView::getGrow() {
    return YGNodeStyleGetFlexGrow(this->ygNode);
}

void UIView::setShrink(float grow) {
    YGNodeStyleSetFlexShrink(this->ygNode, grow);
}

float UIView::getShrink() {
    return YGNodeStyleGetFlexShrink(this->ygNode);
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

}
