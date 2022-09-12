//
//  UIAction.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 01.09.2022.
//

#pragma once

#include <functional>
#include <string>

namespace NXKit {

struct UIAction {
    bool visible;
    bool enabled;
    std::string name;
    std::function<void()> action;
    std::function<bool()> condition;

    UIAction(): UIAction([](){}, "", false, false, []() { return false; }) { }
    UIAction(std::function<void()> action, std::string name = "", bool enabled = true, bool visible = true, std::function<bool()> condition = [](){ return true; }):
        action(action), name(name), visible(visible), enabled(enabled), condition(condition)
    {}
};

}
