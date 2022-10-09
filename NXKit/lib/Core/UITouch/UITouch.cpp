//
//  UITouch.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 08.07.2022.
//

#include <Core/UITouch/UITouch.hpp>
#include <Core/UIView/UIView.hpp>

namespace NXKit {

UITouch::UITouch(unsigned long touchId, Point atPoint, Time timestamp):
touchId(touchId),
absoluteLocation(atPoint),
previousAbsoluteLocation(atPoint),
timestamp(timestamp)
{ }

void UITouch::updateAbsoluteLocation(Point newLocation) {
    previousAbsoluteLocation = absoluteLocation;
    absoluteLocation = newLocation;
}

Point UITouch::locationIn(std::shared_ptr<UIView> view) {
    return window->convert(absoluteLocation, view);
}

Point UITouch::previousLocationIn(std::shared_ptr<UIView> view) {
    return window->convert(previousAbsoluteLocation, view);
}

void UITouch::runTouchActionOnRecognizerHierachy(std::function<void(std::shared_ptr<UIGestureRecognizer>)> action) {
    for (auto recognizer: gestureRecognizers) {
        if (hasBeenCancelledByAGestureRecognizer) return;
        if (!recognizer->isEnabled()) continue;
        action(recognizer);
    }
}

}
