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

#include "CGRect.hpp"
#include <vector>

CGRect::CGRect(Rect value)
    : currentValue(value)
{
}

void CGRect::onReset()
{
    this->tween = tweeny::tween<float, float, float, float>::from(this->currentValue.origin.x, this->currentValue.origin.y, this->currentValue.size.width, this->currentValue.size.height);
}

void CGRect::reset(Rect initialValue)
{
    this->currentValue = initialValue;
    FiniteTicking::reset();
}

void CGRect::reset()
{
    FiniteTicking::reset();
}

void CGRect::onRewind()
{
    auto seek = this->tween.seek(0);
    this->currentValue = Rect(seek[0], seek[1], seek[2], seek[3]);
}

void CGRect::addStep(Rect targetValue, int32_t duration, EasingFunction easing)
{
    this->tween.to(targetValue.origin.x, targetValue.origin.y, targetValue.size.width, targetValue.size.height).during(duration).via(easing);
}

float CGRect::getProgress()
{
    return this->tween.progress();
}

bool CGRect::onUpdate(retro_time_t delta)
{
    // int32_t for stepping works as long as the app goes faster than 0.00001396983 FPS
    // (in which case the delta for a frame wraps in an int32_t)
    auto step = this->tween.step((int32_t)delta);
    this->currentValue = Rect(step[0], step[1], step[2], step[3]);
    return this->tween.progress() < 1.0f;
}

Rect CGRect::getValue()
{
    return this->currentValue;
}

Point CGRect::origin() { return currentValue.origin; }
Size CGRect::size() { return currentValue.size; }

CGRect::operator Rect() const
{
    return this->currentValue;
}

CGRect::operator Rect()
{
    return this->currentValue;
}

void CGRect::operator=(const Rect value)
{
    this->reset(value);
}

bool CGRect::operator==(const Rect value)
{
    return this->currentValue == value;
}
