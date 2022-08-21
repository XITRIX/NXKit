/*
    Copyright 2021 natinusala

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <Core/Utils/Animation/NXPoint/NXPoint.hpp>
#include <vector>

namespace NXKit {

NXPoint::NXPoint(Point value)
: currentValue(value)
{
}

void NXPoint::onReset()
{
    this->tween = tweeny::tween<float, float>::from(this->currentValue.x, this->currentValue.y);
}

void NXPoint::reset(Point initialValue)
{
    this->currentValue = initialValue;
    FiniteTicking::reset();
}

void NXPoint::reset()
{
    FiniteTicking::reset();
}

void NXPoint::onRewind()
{
    auto seek = this->tween.seek(0);
    this->currentValue = Point(seek[0], seek[1]);
}

void NXPoint::addStep(Point targetValue, int32_t duration, EasingFunction easing)
{
    this->tween.to(targetValue.x, targetValue.y).during(duration).via(easing);
}

float NXPoint::getProgress()
{
    return this->tween.progress();
}

bool NXPoint::onUpdate(retro_time_t delta)
{
    // int32_t for stepping works as long as the app goes faster than 0.00001396983 FPS
    // (in which case the delta for a frame wraps in an int32_t)
    auto step = this->tween.step((int32_t)delta);
    this->currentValue = Point(step[0], step[1]);
    return this->tween.progress() < 1.0f;
}

Point NXPoint::getValue()
{
    return this->currentValue;
}

float NXPoint::x()
{
    return this->currentValue.x;
}

float NXPoint::y()
{
    return this->currentValue.y;
}

NXPoint::operator Point() const
{
    return this->currentValue;
}

NXPoint::operator Point()
{
    return this->currentValue;
}

void NXPoint::operator=(const Point value)
{
    this->reset(value);
}

bool NXPoint::operator==(const Point value)
{
    return this->currentValue == value;
}

}
