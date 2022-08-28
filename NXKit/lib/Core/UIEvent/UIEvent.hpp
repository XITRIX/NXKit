//
//  UIEvent.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#pragma once

#include <Core/Utils/Animation/Core/Time.hpp>
#include <vector>
#include <chrono>

namespace NXKit {

class UITouch;

class UIEvent {
public:
    std::vector<UITouch*> allTouches;
    std::time_t timestamp = getCPUTimeUsec();

    UIEvent() {}
    UIEvent(UITouch* touch);
private:
    static std::vector<UIEvent*> activeEvents;
};

}
