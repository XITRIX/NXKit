//
//  SwitchInputManager.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 21.08.2022.
//

#include <Platforms/Switch/SwitchInputManager.hpp>
#include <Core/Application/Application.hpp>
#include <Core/UIView/UIView.hpp>

#include <switch.h>

namespace NXKit {

void SwitchInputManager::updateTouch() {
    HidTouchScreenState hidState;
    
    std::map<int, UITouch*> oldTouches;
    for (auto touch: touches) {
        oldTouches[touch->touchId] = touch;
    }

    if (hidGetTouchScreenStates(&hidState, 1))
    {
        for (int i = 0; i < hidState.count; i++)
        {
            auto find = oldTouches.find(hidState.touches[i].finger_id);
            if (find == oldTouches.end()) {
                auto touch = new UITouch(hidState.touches[i].finger_id, Point(hidState.touches[i].x, hidState.touches[i].y), std::time(nullptr));
                touch->window = Application::shared()->getKeyWindow();
                touch->view = touch->window->hitTest(touch->absoluteLocation, nullptr);
                touch->gestureRecognizers = getRecognizerHierachyFrom(touch->view);
                touch->phase = UITouchPhase::BEGIN;
                touches.push_back(touch);
                // printf("Touch begin at: X - %d, Y - %d\n", hidState.touches[i].x, hidState.touches[i].y);
            } else {
                auto touch = find->second;
                touch->timestamp = std::time(nullptr);
                touch->phase = UITouchPhase::MOVED;
                touch->updateAbsoluteLocation(Point(hidState.touches[i].x, hidState.touches[i].y));
                oldTouches.erase(find);
                // printf("Touch moved at: X - %d, Y - %d\n", hidState.touches[i].x, hidState.touches[i].y);
            }
        }

        for (auto touchIter: oldTouches) {
            UITouch* touch = touchIter.second;
            if (touch->phase == UITouchPhase::MOVED) {
                touch->phase = UITouchPhase::ENDED;
                // printf("Touch ended\n");
            } else if (touch->phase == UITouchPhase::ENDED) {
                touches.erase(std::remove(touches.begin(), touches.end(), touch));
                // printf("Touch removed\n");
                delete touch;
            }
        }
    }
}

}
