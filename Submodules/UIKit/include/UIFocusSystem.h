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
class UIPressesEvent;
class UIFocusSystem {
public:
    UIFocusSystem();

    // Return nullptr if FocusSystem is not active
    std::weak_ptr<UIFocusItem> focusedItem() { return _isActive ? _focusedItem : std::weak_ptr<UIFocusItem>(); }
private:
    void setActive(bool active);
    bool _isActive = false;

    std::weak_ptr<UIFocusItem> _selectedFocusedItem;
    std::weak_ptr<UIFocusItem> _focusedItem;
    std::weak_ptr<UIWindow> _rootWindow;

    void updateFocus(bool resetFocusView = false);
    void sendEvent(const std::shared_ptr<UIEvent>& event);

    std::shared_ptr<UIFocusItem> findItemToFocus();
    void applyFocusToItem(const std::shared_ptr<UIFocusItem>& item, UIFocusUpdateContext context);

    static UIFocusHeading makeFocusHeadingFromEvent(const std::shared_ptr<UIPressesEvent>& event);

    friend class UIWindow;
    friend class UIControl;
};

}
