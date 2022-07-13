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

#include "CGSize.hpp"
#include <vector>

CGSize::CGSize(Size value)
    : currentValue(value)
{
}

void CGSize::onReset()
{
    this->tween = tweeny::tween<float, float>::from(this->currentValue.width, this->currentValue.height);
}

void CGSize::reset(Size initialValue)
{
    this->currentValue = initialValue;
    FiniteTicking::reset();
}

void CGSize::reset()
{
    FiniteTicking::reset();
}

void CGSize::onRewind()
{
    auto seek = this->tween.seek(0);
    this->currentValue = Size(seek[0], seek[1]);
}

void CGSize::addStep(Size targetValue, int32_t duration, EasingFunction easing)
{
    this->tween.to(targetValue.width, targetValue.height).during(duration).via(easing);
}

float CGSize::getProgress()
{
    return this->tween.progress();
}

bool CGSize::onUpdate(retro_time_t delta)
{
    // int32_t for stepping works as long as the app goes faster than 0.00001396983 FPS
    // (in which case the delta for a frame wraps in an int32_t)
    auto step = this->tween.step((int32_t)delta);
    this->currentValue = Size(step[0], step[1]);
    return this->tween.progress() < 1.0f;
}

Size CGSize::getValue()
{
    return this->currentValue;
}

float CGSize::width()
{
    return this->currentValue.width;
}

float CGSize::height()
{
    return this->currentValue.height;
}

CGSize::operator Size() const
{
    return this->currentValue;
}

CGSize::operator Size()
{
    return this->currentValue;
}

void CGSize::operator=(const Size value)
{
    this->reset(value);
}

bool CGSize::operator==(const Size value)
{
    return this->currentValue == value;
}
