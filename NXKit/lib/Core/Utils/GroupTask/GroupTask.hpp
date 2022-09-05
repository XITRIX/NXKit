//
//  GroupTask.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 05.09.2022.
//

#pragma once

#include <functional>

namespace NXKit {

class GroupTask {
public:
    GroupTask(std::function<void()> complete);
    void enter();
    void leave();
private:
    std::function<void()> complete;
    int counter = 0;
};

}
