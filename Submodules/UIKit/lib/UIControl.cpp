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
            setBackgroundColor(UIColor::systemCyan);
            setTransform(NXAffineTransform::scale(1.2f));
        });
    } else {
        coordinator->addCoordinatedAnimations([this]() {
            setBackgroundColor(UIColor::clear);
            setTransform(NXAffineTransform::identity);
        });
    }
}


}

