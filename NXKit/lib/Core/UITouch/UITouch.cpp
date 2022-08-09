//
//  UITouch.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 08.07.2022.
//

#include "UITouch.hpp"
#include "UIView.hpp"

namespace NXKit {

UITouch::UITouch(int touchId, Point atPoint, std::time_t timestamp):
touchId(touchId),
absoluteLocation(atPoint),
previousAbsoluteLocation(atPoint),
timestamp(timestamp)
{ }

void UITouch::updateAbsoluteLocation(Point newLocation) {
    previousAbsoluteLocation = absoluteLocation;
    absoluteLocation = newLocation;
}

Point UITouch::locationIn(UIView* view) {
    return window->convert(absoluteLocation, view);
}

Point UITouch::previousLocationIn(UIView* view) {
    return window->convert(previousAbsoluteLocation, view);
}

void UITouch::runTouchActionOnRecognizerHierachy(std::function<void(UIGestureRecognizer*)> action) {
    for (auto recognizer: gestureRecognizers) {
        if (!recognizer->isEnabled()) continue;
        action(recognizer);
    }
}

}
