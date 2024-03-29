//
//  UIEvent.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#include <Core/UIEvent/UIEvent.hpp>

namespace NXKit {

UIEvent::UIEvent(std::shared_ptr<UITouch> touch) {
    allTouches.push_back(touch);
}

}
