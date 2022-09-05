//
//  GroupTask.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 05.09.2022.
//

#include <Core/Utils/GroupTask/GroupTask.hpp>

namespace NXKit {

GroupTask::GroupTask(std::function<void()> complete):
    complete(complete)
{ }

void GroupTask::enter() {
    counter++;
}

void GroupTask::leave() {
    if (--counter == 0) {
        complete();
        delete this;
    }
}

}
