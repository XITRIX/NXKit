//
//  UIControl.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 03.04.2023.
//

#include <UIControl.h>
#include <UIControlGestureRecognizer.h>

namespace NXKit {

UIControl::UIControl() {
    addGestureRecognizer(new_shared<UIControlGestureRecognizer>(this));
}

bool UIControl::canBecomeFocused() {
    return !isHidden() && alpha() > 0 && isUserInteractionEnabled();
}

void UIControl::didUpdateFocusIn(UIFocusUpdateContext context, UIFocusAnimationCoordinator* coordinator) {
    UIView::didUpdateFocusIn(context, coordinator);

    if (context.nextFocusedItem().lock() == shared_from_this()) {
        coordinator->addCoordinatedAnimations([this]() {
            willGainFocus();
        });
    } else {
        coordinator->addCoordinatedAnimations([this]() {
            willLoseFocus();
        });
    }
}

void UIControl::willGainFocus() {
//            setBackgroundColor(UIColor::systemCyan);
    setTransform(NXAffineTransform::scale(1.02f));
    setBaseScaleMultiplier(1.1f);
    layer()->setBorderColor(UIColor::systemBlue);

    UIView::performWithoutAnimation([this]() {
        layer()->setBorderWidth(3);
    });
}

void UIControl::willLoseFocus() {
//            setBackgroundColor(UIColor::clear);
    setTransform(NXAffineTransform::identity);
    setBaseScaleMultiplier(1);
    layer()->setBorderColor(tintColor());
    UIView::performWithoutAnimation([this]() {
        layer()->setBorderWidth(0);
    });
}

bool UIControl::isEnabled() const {
    return !_state[uint8_t (UIControlState::disabled)];
}

void UIControl::setEnabled(bool enabled) {
     _state[uint8_t (UIControlState::disabled)] = !enabled;
}

bool UIControl::isSelected() const {
    return _state[uint8_t (UIControlState::selected)];
}

void UIControl::setSelected(bool selected) {
    _state[uint8_t (UIControlState::selected)] = selected;
}

bool UIControl::isHighlighted() const {
    return _state[uint8_t (UIControlState::highlighted)];
}

void UIControl::setHighlighted(bool highlighted) {
    _state[uint8_t (UIControlState::highlighted)] = highlighted;
}

void UIControl::setBaseScaleMultiplier(NXFloat baseScaleMultiplier) {
    if (_baseScaleMultiplier == baseScaleMultiplier) return;
    _baseScaleMultiplier = baseScaleMultiplier;
    baseScaleMultiplierDidChange();
}
}

