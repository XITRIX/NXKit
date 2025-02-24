//
//  UIFocusAnimationCoordinator.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 02.04.2023.
//

#include <UIFocusAnimationCoordinator.h>

namespace NXKit {

void UIFocusAnimationCoordinator::addCoordinatedAnimations(std::function<void()> animations, std::function<void()> completion) {
    _coordinatedAnimations.push_back(animations);
    _coordinatedAnimationCompletions.push_back(completion);
}

void UIFocusAnimationCoordinator::addCoordinatedFocusingAnimations(std::function<void(UIFocusAnimationContext)> animations, std::function<void()> completion) {
    _coordinatedFocusingAnimations.push_back(animations);
    _coordinatedAnimationCompletions.push_back(completion);
}

void UIFocusAnimationCoordinator::addCoordinatedUnfocusingAnimations(std::function<void(UIFocusAnimationContext)> animations, std::function<void()> completion) {
    _coordinatedUnfocusingAnimations.push_back(animations);
    _coordinatedAnimationCompletions.push_back(completion);
}

}
