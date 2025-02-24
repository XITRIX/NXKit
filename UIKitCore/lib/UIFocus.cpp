//
//  UIFocusEnvironment.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 02.04.2023.
//

#include <UIFocus.h>

namespace NXKit {

bool UIFocusEnvironment::shouldUpdateFocusIn(UIFocusUpdateContext context) {
    return true;
}

std::vector<std::shared_ptr<UIFocusEnvironment>> UIFocusEnvironment::preferredFocusEnvironments() {
    if (!_lastFocusEnvironment.expired()) return { _lastFocusEnvironment.lock() };
    return {};
}

void UIFocusEnvironment::didUpdateFocusIn(UIFocusUpdateContext context, UIFocusAnimationCoordinator* coordinator) {
    if (!context.nextFocusedItem().expired()) {
        if (parentFocusEnvironment()) {
            std::shared_ptr<UIFocusEnvironment> focus = context.nextFocusedItem().lock();
            while (focus && focus->parentFocusEnvironment()) {
                if (focus->parentFocusEnvironment().get() == this) {
                    _lastFocusEnvironment = focus;
                    break;
                }
                focus = focus->parentFocusEnvironment();
            }
        }
    }

    if (parentFocusEnvironment()) parentFocusEnvironment()->didUpdateFocusIn(context, coordinator);
}

}
