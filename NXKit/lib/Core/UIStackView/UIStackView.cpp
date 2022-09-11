//
//  UIStackView.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 05.07.2022.
//

#include <Core/UIStackView/UIStackView.hpp>

namespace NXKit {

UIStackView::UIStackView(Axis axis): UIView() {
    setAxis(axis);
    //    YGNodeStyleSetJustifyContent(this->ygNode, YGJustify::YGJustifyCenter);
}

UIStackView::UIStackView(Rect frame): UIView(frame) {
    setAxis(Axis::VERTICAL);
    //    YGNodeStyleSetJustifyContent(this->ygNode, YGJustify::YGJustifyCenter);
}

void UIStackView::addSubview(UIView* view) {
    YGNodeInsertChild(this->ygNode, view->getYGNode(), (int) getSubviews().size());
    UIView::addSubview(view);
}

void UIStackView::setAxis(Axis axis) {
    this->axis = axis;
    switch (axis) {
        case Axis::VERTICAL:
            YGNodeStyleSetFlexDirection(this->ygNode, YGFlexDirectionColumn);
            break;
        case Axis::HORIZONTAL:
            YGNodeStyleSetFlexDirection(this->ygNode, YGFlexDirectionRow);
            break;
    }
    setNeedsLayout();
}

UIView* UIStackView::getNextFocus(NavigationDirection direction) {
    if (currentFocus >= getSubviews().size()) currentFocus = (int) getSubviews().size() - 1;

    if ((direction == NavigationDirection::UP || direction == NavigationDirection::DOWN) && axis == Axis::HORIZONTAL)
        return UIView::getNextFocus(direction);

    if ((direction == NavigationDirection::LEFT || direction == NavigationDirection::RIGHT) && axis == Axis::VERTICAL)
        return UIView::getNextFocus(direction);

    if (direction == NavigationDirection::UP || direction == NavigationDirection::LEFT) {
        float newFocus = currentFocus;
        UIView* newFocusView = nullptr;

        do {
            newFocus--;
            if (newFocus < 0) return UIView::getNextFocus(direction);
            newFocusView = getSubviews()[newFocus]->getDefaultFocus();
        } while (!newFocusView || !newFocusView->canBecomeFocused());

        if (newFocusView && newFocusView->canBecomeFocused()) {
            currentFocus = newFocus;
            return newFocusView;
        }
    }

    if (direction == NavigationDirection::DOWN || direction == NavigationDirection::RIGHT) {
        float newFocus = currentFocus;
        UIView* newFocusView = nullptr;

        do {
            newFocus++;
            if (newFocus >= getSubviews().size()) return UIView::getNextFocus(direction);
            newFocusView = getSubviews()[newFocus]->getDefaultFocus();
        } while (!newFocusView || !newFocusView->canBecomeFocused());

        if (newFocusView && newFocusView->canBecomeFocused()) {
            currentFocus = newFocus;
            return newFocusView;
        }
    }

    return UIView::getNextFocus(direction);
}

UIView* UIStackView::getDefaultFocus() {
    if (canBecomeFocused()) return this;

    auto subviews = getSubviews();

    if (subviews.size() > currentFocus) {
        auto currentSelection = getSubviews()[currentFocus]->getDefaultFocus();
        if (currentSelection)
            return currentSelection;
    }

    return UIView::getDefaultFocus();
}

UIView* UIStackView::hitTest(Point point, UIEvent *withEvent) {
    auto super = UIView::hitTest(point, withEvent);
    if (super == this && passthroughTouches)
        return nullptr;
    return super;
}

void UIStackView::subviewFocusDidChange(UIView *focusedView, UIView *notifiedView) {
    auto subviews = getSubviews();
    auto find = std::find(subviews.begin(), subviews.end(), notifiedView);
    if (find != subviews.end())
        currentFocus = (int) (find - subviews.begin());
    UIView::subviewFocusDidChange(focusedView, notifiedView);
}

void UIStackView::setJustifyContent(JustifyContent justify)
{
    switch (justify)
    {
        case JustifyContent::FLEX_START:
            YGNodeStyleSetJustifyContent(this->ygNode, YGJustifyFlexStart);
            break;
        case JustifyContent::CENTER:
            YGNodeStyleSetJustifyContent(this->ygNode, YGJustifyCenter);
            break;
        case JustifyContent::FLEX_END:
            YGNodeStyleSetJustifyContent(this->ygNode, YGJustifyFlexEnd);
            break;
        case JustifyContent::SPACE_BETWEEN:
            YGNodeStyleSetJustifyContent(this->ygNode, YGJustifySpaceBetween);
            break;
        case JustifyContent::SPACE_AROUND:
            YGNodeStyleSetJustifyContent(this->ygNode, YGJustifySpaceAround);
            break;
        case JustifyContent::SPACE_EVENLY:
            YGNodeStyleSetJustifyContent(this->ygNode, YGJustifySpaceEvenly);
            break;
    }
    
    this->setNeedsLayout();
}

void UIStackView::setAlignItems(AlignItems alignment)
{
    switch (alignment)
    {
        case AlignItems::AUTO:
            YGNodeStyleSetAlignItems(this->ygNode, YGAlignAuto);
            break;
        case AlignItems::FLEX_START:
            YGNodeStyleSetAlignItems(this->ygNode, YGAlignFlexStart);
            break;
        case AlignItems::CENTER:
            YGNodeStyleSetAlignItems(this->ygNode, YGAlignCenter);
            break;
        case AlignItems::FLEX_END:
            YGNodeStyleSetAlignItems(this->ygNode, YGAlignFlexEnd);
            break;
        case AlignItems::STRETCH:
            YGNodeStyleSetAlignItems(this->ygNode, YGAlignStretch);
            break;
        case AlignItems::BASELINE:
            YGNodeStyleSetAlignItems(this->ygNode, YGAlignBaseline);
            break;
        case AlignItems::SPACE_BETWEEN:
            YGNodeStyleSetAlignItems(this->ygNode, YGAlignSpaceBetween);
            break;
        case AlignItems::SPACE_AROUND:
            YGNodeStyleSetAlignItems(this->ygNode, YGAlignSpaceAround);
            break;
    }
    
    this->setNeedsLayout();
}

void UIStackView::setPadding(float top, float right, float bottom, float left) {
    YGNodeStyleSetPadding(this->ygNode, YGEdgeTop, top);
    YGNodeStyleSetPadding(this->ygNode, YGEdgeRight, right);
    YGNodeStyleSetPadding(this->ygNode, YGEdgeBottom, bottom);
    YGNodeStyleSetPadding(this->ygNode, YGEdgeLeft, left);
    this->setNeedsLayout();
}

void UIStackView::setPaddingTop(float top) {
    YGNodeStyleSetPadding(this->ygNode, YGEdgeTop, top);
    this->setNeedsLayout();
}

void UIStackView::setPaddingLeft(float left) {
    YGNodeStyleSetPadding(this->ygNode, YGEdgeLeft, left);
    this->setNeedsLayout();
}

void UIStackView::setPaddingRight(float right) {
    YGNodeStyleSetPadding(this->ygNode, YGEdgeRight, right);
    this->setNeedsLayout();
}

void UIStackView::setPaddingBottom(float bottom) {
    YGNodeStyleSetPadding(this->ygNode, YGEdgeBottom, bottom);
    this->setNeedsLayout();
}

}
