//
//  UIView.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include <Core/UIView/UIView.hpp>
#include <Core/UIWindow/UIWindow.hpp>
#include <Core/UIViewController/UIViewController.hpp>
#include <Core/Application/Application.hpp>
#include <Core/UIStackView/UIStackView.hpp>
#include <Platforms/InputManager.hpp>

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
    bounds.size = size;
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

void UIView::setWidth(float width) {
    bounds.size.width = width;
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
    bounds.size.height = height;
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
    Rect bounds;
    bounds.origin = this->bounds.origin;
    bounds.size = Size(YGNodeLayoutGetWidth(this->ygNode), YGNodeLayoutGetHeight(this->ygNode));

//    bounds.size = Size(this->bounds.width() == AUTO ? YGNodeLayoutGetWidth(this->ygNode) : this->bounds.width(),
//                       this->bounds.height() == AUTO ? YGNodeLayoutGetHeight(this->ygNode) : this->bounds.height());

    return bounds;
}

void UIView::setBounds(Rect bounds) {
    this->bounds = bounds;
}

void UIView::internalDraw(NVGcontext* vgContext) {
    if (isHidden()) return;

    nvgSave(vgContext);
    layoutIfNeeded();

    // Frame position
    nvgTranslate(vgContext, getFrame().origin.x , getFrame().origin.y);

    // Position transform
    nvgTranslate(vgContext, transformOrigin.x() , transformOrigin.y());

    // Scale transform
    float scaleTransformX = (getFrame().size.width / 2.0f) - (getFrame().size.width / 2.0f) * transformSize.width();
    float scaleTransformY = (getFrame().size.height / 2.0f) - (getFrame().size.height / 2.0f) * transformSize.height();
    nvgTranslate(vgContext, scaleTransformX, scaleTransformY);
    nvgScale(vgContext, transformSize.width(), transformSize.height());

    // ClipToBounds
    if (clipToBounds)
        nvgIntersectScissor(vgContext, 0, 0, getFrame().size.width, getFrame().size.height);

    // Background color
    nvgBeginPath(vgContext);
    nvgRoundedRect(vgContext, 0, 0, getFrame().size.width, getFrame().size.height, cornerRadius);
    nvgFillColor(vgContext, backgroundColor.raw());
    nvgFill(vgContext);

    nvgSave(vgContext);
    nvgTranslate(vgContext, -bounds.minX(), -bounds.minY());
    draw(vgContext);

    nvgSave(vgContext);
    //    nvgTranslate(vgContext, getFrame().origin.x, getFrame().origin.y);
    for (auto view: subviews) {
        view->internalDraw(vgContext);
    }
    nvgRestore(vgContext);
    nvgRestore(vgContext);

//    nvgTranslate(vgContext, getFrame().origin.x , getFrame().origin.y);
    // Borders
    if (borderThickness > 0) {
        float offset = borderThickness / 2;
        Rect borderRect = Rect(offset, offset, getFrame().size.width - offset * 2, getFrame().size.height - offset * 2);

        nvgBeginPath(vgContext);
        nvgStrokeColor(vgContext, borderColor.raw());
        nvgStrokeWidth(vgContext, this->borderThickness);
        nvgRoundedRect(vgContext, borderRect.origin.x, borderRect.origin.y, borderRect.size.width, borderRect.size.height, this->cornerRadius);
        nvgStroke(vgContext);
    }

    if (getBorderTop() > 0) {
        nvgBeginPath(vgContext);
        nvgRect(vgContext, 0, 0, getFrame().size.width, getBorderTop());
        nvgFillColor(vgContext, borderColor.raw());
        nvgFill(vgContext);
    }

    if (getBorderLeft() > 0) {
        nvgBeginPath(vgContext);
        nvgRect(vgContext, 0, 0, getBorderLeft(), getFrame().size.height);
        nvgFillColor(vgContext, borderColor.raw());
        nvgFill(vgContext);
    }

    if (getBorderRight() > 0) {
        nvgBeginPath(vgContext);
        nvgRect(vgContext, (0 + getFrame().size.width - getBorderRight()), 0, getBorderRight(), getFrame().size.height);
        nvgFillColor(vgContext, borderColor.raw());
        nvgFill(vgContext);
    }

    if (getBorderBottom() > 0) {
        nvgBeginPath(vgContext);
        nvgRect(vgContext, 0, (getFrame().size.height - getBorderBottom()), getFrame().size.width, getBorderBottom());
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
        auto viewParentNode = YGNodeGetParent(view->ygNode);
        if (!viewParentNode || viewParentNode != this->ygNode) continue;
        view->layoutSubviews();
    }

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
    gestureRecognizer->view = this;
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

void UIView::removeFromSuperview() {
    if (superview)
        superview->subviews.erase(std::remove(superview->subviews.begin(), superview->subviews.end(), this));

    YGNodeRemoveChild(superview->ygNode, ygNode);
    superview = nullptr;
}

Point UIView::convert(Point point, UIView* toView) {
    Point selfAbsoluteOrigin;
    Point otherAbsoluteOrigin;

    UIView* current = this;
    while (current) {
        if (current == toView) {
            return point - selfAbsoluteOrigin;
        }
        selfAbsoluteOrigin += current->getFrame().origin;
        selfAbsoluteOrigin -= current->bounds.origin;
        current = current->superview;
    }

    current = toView;
    while (current) {
        otherAbsoluteOrigin += current->getFrame().origin;
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

    if (dynamic_cast<UIStackView*>(this) && !this->canBecomeFocused)
        return nullptr;

    return this;
}

bool UIView::point(Point insidePoint, UIEvent *withEvent) {
    return bounds.contains(insidePoint);
}

UIEdgeInsets UIView::safeAreaInsets() {
    auto insets = UIEdgeInsets::zero;
    if (controller && controller->getParent()) {
        insets += controller->getParent()->getAdditionalSafeAreaInsets();
    }
    if (superview) {
        insets += superview->safeAreaInsets();

        if (getFrame().origin.y > 0 && insets.top > 0) {
            insets.top -= getFrame().origin.y;
            if (insets.top < 0) insets.top = 0;
        }

        if (getFrame().origin.x > 0 || insets.left > 0) {
            insets.left -= getFrame().origin.x;
            if (insets.left < 0) insets.left = 0;
        }

        auto right = superview->getBounds().size.width - getFrame().origin.x - getBounds().size.width;
        if (right > 0 || insets.right > 0) {
            insets.right -= right;
            if (insets.right < 0) insets.right = 0;
        }

        auto bottom = superview->getBounds().size.height - getFrame().origin.y - getBounds().size.height;
        if (bottom > 0 || insets.bottom > 0) {
            insets.bottom -= bottom;
            if (insets.bottom < 0) insets.bottom = 0;
        }
    }

    return insets;
}

UIWindow* UIView::getWindow() {
    auto superview = this;
    while (superview) {
        auto cast = dynamic_cast<UIWindow*>(superview);
        if (cast) return cast;
        superview = superview->getSuperview();
    }
    return nullptr;
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
