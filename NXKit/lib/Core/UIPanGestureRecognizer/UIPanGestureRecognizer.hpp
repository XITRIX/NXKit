//
//  UIPanGestureRecognizer.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 20.08.2022.
//

#pragma once

#include <Core/UIGestureRecognizer/UIGestureRecognizer.hpp>
#include <Core/UITouch/UITouch.hpp>
#include <functional>

namespace NXKit {

class UIPanGestureRecognizer: public UIGestureRecognizer {
public:
    void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;

    Point translationInView(std::shared_ptr<UIView> view);
    void setTranslation(Point translation, std::shared_ptr<UIView> inView);

    Point velocityIn(std::shared_ptr<UIView> view);

private:
    std::shared_ptr<UITouch> trackingTouch;
    Point initialTouchPoint;
    time_t touchesMovedTimestamp;
    time_t previousTouchesMovedTimestamp;

    Point velocityIn(std::shared_ptr<UIView> view, float timeDiffSeconds);
};

}
