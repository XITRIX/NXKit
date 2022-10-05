//
//  UITouch.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 08.07.2022.
//

#pragma once

#include <Core/Utils/Animation/Core/Time.hpp>
#include <Core/Geometry/Geometry.hpp>
#include <Core/UIGestureRecognizer/UIGestureRecognizer.hpp>

#include <ctime>
#include <vector>
#include <functional>

namespace NXKit {

enum class UITouchPhase {
    BEGIN,
    MOVED,
    ENDED,
};

class UIView;

class UITouch {
public:
    UITouch(unsigned long touchId, Point atPoint, Time timestamp);
    UITouch(): UITouch(0, Point(), getCPUTimeUsec()) {}

    unsigned long touchId;

    UIView* view = nullptr;
    UIView* window = nullptr;

    UITouchPhase phase = UITouchPhase::BEGIN;
    std::time_t timestamp;

    Point absoluteLocation;
    Point previousAbsoluteLocation;

    void updateAbsoluteLocation(Point newLocation);

    Point locationIn(UIView* view);
    Point previousLocationIn(UIView* view);

    std::vector<UIGestureRecognizer*> gestureRecognizers;
    void runTouchActionOnRecognizerHierachy(std::function<void(UIGestureRecognizer*)> action);

    bool hasBeenCancelledByAGestureRecognizer = false;
};

constexpr bool operator==(const UITouch& lhs, const UITouch& rhs) {
    return lhs.touchId == rhs.touchId;
}

}
