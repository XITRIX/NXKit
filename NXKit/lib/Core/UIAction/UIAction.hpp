//
//  UIAction.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 01.09.2022.
//

#pragma once

#include <functional>

namespace NXKit {

struct UIAction {
    bool visible;
    bool enabled;
    std::function<void()> action;

    UIAction(): UIAction([](){}) { }
    UIAction(std::function<void()> action, bool visible = true, bool enabled = true):
        action(action), visible(visible), enabled(enabled)
    {}
};

}
