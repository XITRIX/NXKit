//
//  UITableViewSwitchCell.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 13.09.2022.
//

#pragma once

#include <Core/UITableViewDefaultCell/UITableViewDefaultCell.hpp>

namespace NXKit {

class UITableViewSwitchCell: public UITableViewDefaultCell {
public:
    UITableViewSwitchCell();

    void setOn(bool on, bool animated = true);
    bool isOn() { return state; }

    void tintColorDidChange() override;

private:
    bool state;

    void updateUI();
};

}
