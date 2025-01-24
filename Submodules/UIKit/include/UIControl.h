//
//  UIControl.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 03.04.2023.
//

#pragma once

#include <UIView.h>
#include <bitset>

namespace NXKit {

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
    UIControl();

    bool canBecomeFocused() override;
    void didUpdateFocusIn(UIFocusUpdateContext context, UIFocusAnimationCoordinator* coordinator) override;

    static std::shared_ptr<UIView> init() { return new_shared<UIControl>(); }

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isSelected() const;
    void setSelected(bool selected);

    bool isHighlighted() const;
    void setHighlighted(bool highlighted);

    virtual void willGainFocus();
    virtual void willLoseFocus();

    void setBaseScaleMultiplier(NXFloat baseScaleMultiplier);
    [[nodiscard]] NXFloat baseScaleMultiplier() const { return _baseScaleMultiplier; }
    virtual void baseScaleMultiplierDidChange() {}
private:
    std::bitset<8> _state;
    NXFloat _baseScaleMultiplier = 1;
};

}
