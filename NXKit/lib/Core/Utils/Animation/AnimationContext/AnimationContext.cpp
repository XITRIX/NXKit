//
//  AnimationContext.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 27.08.2022.
//

#include <Core/Utils/Animation/AnimationContext/AnimationContext.hpp>
#include <vector>

namespace NXKit {

AnimationContext::AnimationContext()
{
}

void AnimationContext::onReset()
{
    this->tweens.clear();
    for (int i = 0; i < this->currentValues.size(); i++)
        this->tweens.push_back(tweeny::tween<float>::from(this->currentValues[i]));
}

void AnimationContext::reset(std::vector<float> initialValue)
{
    this->currentValues = initialValue;
    FiniteTicking::reset();
}

void AnimationContext::reset()
{
    FiniteTicking::reset();
}

void AnimationContext::onRewind()
{
    for (int i = 0; i < this->tweens.size(); i++) {
        this->currentValues[i] = this->tweens[i].seek(0);
    }
}

void AnimationContext::addStep(std::vector<float> targetValue, int32_t duration, EasingFunction easing)
{
    if (currentValues.size() != targetValue.size()) {
        printf("FATAL ERROR!\n");
        return;
    }

    for (int i = 0; i < targetValue.size(); i++)
        this->tweens[i].to(targetValue[i]).during(duration).via(easing);
}

float AnimationContext::getProgress()
{
    return this->tweens.front().progress();
}

bool AnimationContext::onUpdate(retro_time_t delta)
{
    // int32_t for stepping works as long as the app goes faster than 0.00001396983 FPS
    // (in which case the delta for a frame wraps in an int32_t)
    bool res = false;
    for (int i = 0; i < currentValues.size(); i++) {
        this->currentValues[i] = this->tweens[i].step((int32_t)delta);
        res |= this->tweens[i].progress() < 1.0f;
    }
    return res;
}

std::vector<float> AnimationContext::getValue()
{
    return this->currentValues;
}

}
