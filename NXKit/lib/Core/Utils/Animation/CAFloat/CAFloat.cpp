//
//  CAFloat.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 27.08.2022.
//

#include <Core/Utils/Animation/CAFloat/CAFloat.hpp>
#include <vector>

namespace NXKit
{

CAFloat::CAFloat(float value)
    : currentValue(value)
{
}

void CAFloat::onReset()
{
    this->tween = tweeny::tween<float>::from(this->currentValue);
}

void CAFloat::reset(float initialValue)
{
    this->currentValue = initialValue;
    FiniteTicking::reset();
}

void CAFloat::reset()
{
    FiniteTicking::reset();
}

void CAFloat::onRewind()
{
    this->currentValue = this->tween.seek(0);
}

void CAFloat::addStep(float targetValue, int32_t duration, EasingFunction easing)
{
    this->tween.to(targetValue).during(duration).via(easing);
}

float CAFloat::getProgress()
{
    return this->tween.progress();
}

bool CAFloat::onUpdate(retro_time_t delta)
{
    // int32_t for stepping works as long as the app goes faster than 0.00001396983 FPS
    // (in which case the delta for a frame wraps in an int32_t)
    this->currentValue = this->tween.step((int32_t)delta);
    return this->tween.progress() < 1.0f;
}

float CAFloat::getValue()
{
    return this->currentValue;
}

CAFloat::operator float() const
{
    return this->currentValue;
}

CAFloat::operator float()
{
    return this->currentValue;
}

void CAFloat::operator=(const float value)
{
    this->reset(value);
}

bool CAFloat::operator==(const float value)
{
    return this->currentValue == value;
}

}
