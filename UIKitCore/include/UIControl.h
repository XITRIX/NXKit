//
//  UIControl.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 03.04.2023.
//

#pragma once

#include <UIView.h>
#include <UIAction.h>
#include <bitset>

namespace NXKit {

enum class UIControlEvent: uint {
    touchDown = 1 << 0,
    touchDownRepeat = 1 << 1,
    touchDragInside = 1 << 2,
    touchDragOutside = 1 << 3,
    touchDragEnter = 1 << 4,
    touchDragExit = 1 << 5,
    touchUpInside = 1 << 6,
    touchUpOutside = 1 << 7,
    touchCancel = 1 << 8,
    valueChanged = 1 << 9,
    primaryActionTriggered = 1 << 10,
    menuActionTriggered = 1 << 11,
    editingDidBegin = 1 << 12,
    editingChanged = 1 << 13,
    editingDidEnd = 1 << 14,
    editingDidEndOnExit = 1 << 15,
    allTouchEvents = touchDown | touchDownRepeat | touchDragInside | touchDragOutside | touchDragEnter | touchDragExit | touchUpInside | touchUpOutside | touchCancel,
    allEditingEvents = editingDidBegin | editingChanged | editingDidEnd | editingDidEndOnExit,
    applicationReserved = 1 << 18,
    systemReserved = 1 << 19,
    allEvents = allTouchEvents | allEditingEvents | valueChanged | primaryActionTriggered | menuActionTriggered
};

enum class UIControlState: uint8_t {
    normal = 0,
    highlighted = 1,
    disabled = 2,
    selected = 3,
    focused = 4,
    application = 5,
    reserved = 6,
};

class UIControl: public UIView {
public:
    static std::shared_ptr<UIView> init() { return new_shared<UIControl>(); }
    UIControl();

    bool canBecomeFocused() override;
    void didUpdateFocusIn(UIFocusUpdateContext context, UIFocusAnimationCoordinator* coordinator) override;

    void pressesBegan(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) override;
    void pressesEnded(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) override;

    bool isEnabled() const;
    virtual void setEnabled(bool enabled);

    bool isSelected() const;
    virtual void setSelected(bool selected);

    bool isHighlighted() const;
    virtual void setHighlighted(bool highlighted);

    virtual void willGainFocus();
    virtual void willLoseFocus();
    virtual void focusFailedToMove(UIFocusHeading heading);
    virtual void willChangeFocusHighlight(bool highlighted);

    virtual void willChangeHighlight(bool highlighted);
    std::shared_ptr<UIView> hitTest(NXPoint point, UIEvent *withEvent) override;

    void setBaseScaleMultiplier(NXFloat baseScaleMultiplier);
    [[nodiscard]] NXFloat baseScaleMultiplier() const { return _baseScaleMultiplier; }
    virtual void baseScaleMultiplierDidChange() {}

    void performPrimaryAction();

    // Temporary solution, need to rework on addAction(_, for:)
    std::optional<UIAction> primaryAction;
private:
    std::bitset<8> _state;
    NXFloat _baseScaleMultiplier = 1;
};

}
