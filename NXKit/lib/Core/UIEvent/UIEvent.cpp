//
//  UIEvent.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#include "UIEvent.hpp"

UIEvent::UIEvent(UITouch* touch) {
    allTouches.push_back(touch);
}
