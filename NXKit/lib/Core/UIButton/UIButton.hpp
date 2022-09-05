//
//  UIButton.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 04.09.2022.
//

#pragma once

#include <Core/UIControl/UIControl.hpp>
#include <Core/UILabel/UILabel.hpp>

namespace NXKit {

enum class UIButtonStyle {
    DEFAULT,
    PRIMARY,
    HIGHLIGHT,
    BORDERED,
    BORDERLESS,
};

class UIButton: public UIControl {
public:
    UIButton(std::string title = "");

    void becomeFocused() override;
    void resignFocused() override;

    void setStyle(UIButtonStyle style);

    void setTitle(std::string title);
protected:
    UILabel* label;
    UIButtonStyle style;
};

}
