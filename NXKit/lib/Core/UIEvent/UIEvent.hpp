//
//  UIEvent.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#pragma once

#include <vector>

class UITouch;

class UIEvent {
public:
    std::vector<UITouch*> allTouches;
    std::time_t timestamp = time(nullptr);

    UIEvent() {}
    UIEvent(UITouch* touch);
private:
    static std::vector<UIEvent*> activeEvents;
};
