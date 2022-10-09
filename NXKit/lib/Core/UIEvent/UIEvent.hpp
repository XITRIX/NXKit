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
    std::vector<std::shared_ptr<UITouch>> allTouches;
    std::time_t timestamp = getCPUTimeUsec();

    UIEvent() {}
    UIEvent(std::shared_ptr<UITouch> touch);
private:
    static std::vector<std::shared_ptr<UIEvent>> activeEvents;
};

}
