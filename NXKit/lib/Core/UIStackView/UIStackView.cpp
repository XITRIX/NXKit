//
//  UIStackView.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 05.07.2022.
//

#include "UIStackView.hpp"

UIStackView::UIStackView(Axis axis): UIView() {
    setAxis(axis);
//    YGNodeStyleSetJustifyContent(this->ygNode, YGJustify::YGJustifyCenter);
}

void UIStackView::addSubview(UIView* view) {
    YGNodeInsertChild(this->ygNode, view->getYGNode(), (int) getSubviews().size());
    UIView::addSubview(view);
}

void UIStackView::setAxis(Axis axis) {
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
