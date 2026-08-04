//
//  UIFocusSystem.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 02.04.2023.
//

#pragma once

#include <UIFocus.h>
#include <UIEvent.h>

#include <utility>
#include <vector>

namespace NXKit {

class UIView;
class UIWindow;
class UIPress;
class UIPressesEvent;
class UIFocusSystem {
public:
    UIFocusSystem();

    // Return nullptr if FocusSystem is not active
    std::weak_ptr<UIFocusItem> focusedItem() { return _isActive ? _focusedItem : std::weak_ptr<UIFocusItem>(); }

    // Requests an explicit focus update within this system's window. A view
    // environment resolves to its first eligible descendant.
    bool requestFocusUpdate(const std::shared_ptr<UIFocusEnvironment>& environment);
private:
    void setActive(bool active);
    bool _isActive = true;

    std::weak_ptr<UIFocusItem> _selectedFocusedItem;
    std::weak_ptr<UIFocusItem> _focusedItem;
    std::weak_ptr<UIWindow> _rootWindow;
    std::vector<std::pair<std::weak_ptr<UIPress>, UIFocusHeading>>
        _pressedFocusHeadings;

    void updateFocus();
    void sendEvent(const std::shared_ptr<UIEvent>& event);
    bool shouldAllowFocusUpdate(UIFocusUpdateContext context);
    void updatePressedFocusHeadings(
        const std::shared_ptr<UIPressesEvent>& event
    );
    bool isFocusHeadingPressed(UIFocusHeading heading);
    bool requestExactFocusUpdate(const std::shared_ptr<UIFocusItem>& item);

    std::shared_ptr<UIFocusItem> findItemToFocus();
    void applyFocusToItem(const std::shared_ptr<UIFocusItem>& item, UIFocusUpdateContext context);

    static UIFocusHeading makeFocusHeadingFromEvent(const std::shared_ptr<UIPressesEvent>& event);

    friend class UIWindow;
    friend class UIControl;
    friend class UIScrollView;
};

}
