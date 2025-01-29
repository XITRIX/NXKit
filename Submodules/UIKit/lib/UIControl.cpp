//
//  UIControl.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 03.04.2023.
//

#include <UIControl.h>
#include <UIControlGestureRecognizer.h>
#include <UIPress.h>
#include <UIWindow.h>

namespace NXKit {

UIControl::UIControl() {
    addGestureRecognizer(new_shared<UIControlGestureRecognizer>(weak_from_base<UIControl>()));
}

bool UIControl::canBecomeFocused() {
    return !isHidden() && alpha() > 0 && isUserInteractionEnabled();
}

void UIControl::pressesBegan(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    UIView::pressesBegan(pressees, event);

    if (std::any_of(pressees.begin(), pressees.end(), [&](const std::shared_ptr<UIPress>& item) {
        return item->type() == UIPressType::select;
    })) {
        setHighlighted(true);
    }
}

void UIControl::pressesEnded(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    UIView::pressesEnded(pressees, event);

    if (std::any_of(pressees.begin(), pressees.end(), [&](const std::shared_ptr<UIPress>& item) {
        return item->type() == UIPressType::select;
    })) {
        if (isHighlighted()) {
            setHighlighted(false);
            performPrimaryAction();
        }
    }
}

void UIControl::didUpdateFocusIn(UIFocusUpdateContext context, UIFocusAnimationCoordinator* coordinator) {
    UIView::didUpdateFocusIn(context, coordinator);

    if (context.nextFocusedItem().lock() == shared_from_this()) {
        coordinator->addCoordinatedAnimations([this]() {
            willGainFocus();
        });
    } else {
        coordinator->addCoordinatedAnimations([this]() {
            _state[uint8_t (UIControlState::highlighted)] = false; // need to be revised
            willLoseFocus();
        });
    }
}

void UIControl::willGainFocus() {
    setTransform(NXAffineTransform::scale(1.06f));
    setBaseScaleMultiplier(1.06f);
    layer()->setShadowColor(UIColor::black);
    layer()->setShadowOpacity(0.4);
    layer()->setShadowOffset({0, 6});
    layer()->setShadowRadius(18);
    layer()->setZPosition(1);

//    UIView::performWithoutAnimation([this]() {
//        layer()->setBorderWidth(3);
//    });
}

void UIControl::willLoseFocus() {
    setTransform(NXAffineTransform::identity);
    setBaseScaleMultiplier(1);
    layer()->setShadowOpacity(0);
    layer()->setShadowRadius(0);
    layer()->setZPosition(0);
//    UIView::performWithoutAnimation([this]() {
//        layer()->setBorderWidth(0);
//    });
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
    if (UIView::isFocused()) {
        UIView::animate(0.2, [&]() {
            if (highlighted) {
                setTransform(NXAffineTransform::scale(1.02f));
                setBaseScaleMultiplier(1.02f);
                layer()->setShadowOffset({0, 3});
                layer()->setShadowRadius(4);
            } else {
                setTransform(NXAffineTransform::scale(1.06f));
                setBaseScaleMultiplier(1.06f);
                layer()->setShadowOffset({0, 6});
                layer()->setShadowRadius(18);
            }
        });
    } else {
        UIView::animate(0.2, 0, UIViewAnimationOptions::allowUserInteraction, [&]() {
            setAlpha(highlighted ? 0.8 : 1);
        });
    }

    _state[uint8_t (UIControlState::highlighted)] = highlighted;
}

void UIControl::setBaseScaleMultiplier(NXFloat baseScaleMultiplier) {
    if (_baseScaleMultiplier == baseScaleMultiplier) return;
    _baseScaleMultiplier = baseScaleMultiplier;
    baseScaleMultiplierDidChange();
}

void UIControl::performPrimaryAction() {
    if (canBecomeFocused())
        window()->focusSystem()->_focusedItem = shared_from_this();

    if (primaryAction.has_value()) {
        primaryAction->_handler();
    }
}
}

