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
#include <Core/Utils/SharedBase/SharedBase.hpp>

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

class UITouch: public enable_shared_from_base<UITouch> {
public:
    UITouch(unsigned long touchId, Point atPoint, Time timestamp);
    UITouch(): UITouch(0, Point(), getCPUTimeUsec()) {}
    virtual ~UITouch() {}

    unsigned long touchId;

    std::shared_ptr<UIView> view;
    std::shared_ptr<UIView> window;

    UITouchPhase phase = UITouchPhase::BEGIN;
    std::time_t timestamp;

    Point absoluteLocation;
    Point previousAbsoluteLocation;

    void updateAbsoluteLocation(Point newLocation);

    Point locationIn(std::shared_ptr<UIView> view);
    Point previousLocationIn(std::shared_ptr<UIView> view);

    std::vector<std::weak_ptr<UIGestureRecognizer>> gestureRecognizers;
    void runTouchActionOnRecognizerHierachy(std::function<void(std::shared_ptr<UIGestureRecognizer>)> action);

    bool hasBeenCancelledByAGestureRecognizer = false;
};

constexpr bool operator==(const UITouch& lhs, const UITouch& rhs) {
    return lhs.touchId == rhs.touchId;
}

}
