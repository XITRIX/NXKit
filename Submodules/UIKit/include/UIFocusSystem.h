//
//  UIFocusSystem.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 02.04.2023.
//

#pragma once

#include <UIFocus.h>
#include <UIEvent.h>

namespace NXKit {

class UIView;
class UIWindow;
class UIFocusSystem {
public:
    UIFocusSystem();
    std::weak_ptr<UIFocusItem> focusedItem() { return _focusedItem; }
private:
    std::weak_ptr<UIFocusItem> _focusedItem;
    std::weak_ptr<UIWindow> _rootWindow;

    void updateFocus();
    void sendEvent(const std::shared_ptr<UIEvent>& event);

    std::shared_ptr<UIFocusItem> findItemToFocus();
    void applyFocusToItem(const std::shared_ptr<UIFocusItem>& item, UIFocusUpdateContext context);

    static UIFocusHeading makeFocusHeadingFromEvent(const std::shared_ptr<UIEvent>& event);

    friend class UIWindow;
};

}
