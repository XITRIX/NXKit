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

#include <Core/Utils/Animation/NXSize/NXSize.hpp>
#include <vector>

namespace NXKit {

NXSize::NXSize(Size value)
: currentValue(value)
{
}

void NXSize::onReset()
{
    this->tween = tweeny::tween<float, float>::from(this->currentValue.width, this->currentValue.height);
}

void NXSize::reset(Size initialValue)
{
    this->currentValue = initialValue;
    FiniteTicking::reset();
}

void NXSize::reset()
{
    FiniteTicking::reset();
}

void NXSize::onRewind()
{
    auto seek = this->tween.seek(0);
    this->currentValue = Size(seek[0], seek[1]);
}

void NXSize::addStep(Size targetValue, int32_t duration, EasingFunction easing)
{
    this->tween.to(targetValue.width, targetValue.height).during(duration).via(easing);
}

float NXSize::getProgress()
{
    return this->tween.progress();
}

bool NXSize::onUpdate(retro_time_t delta)
{
    // int32_t for stepping works as long as the app goes faster than 0.00001396983 FPS
    // (in which case the delta for a frame wraps in an int32_t)
    auto step = this->tween.step((int32_t)delta);
    this->currentValue = Size(step[0], step[1]);
    return this->tween.progress() < 1.0f;
}

Size NXSize::getValue()
{
    return this->currentValue;
}

float NXSize::width()
{
    return this->currentValue.width;
}

float NXSize::height()
{
    return this->currentValue.height;
}

NXSize::operator Size() const
{
    return this->currentValue;
}

NXSize::operator Size()
{
    return this->currentValue;
}

void NXSize::operator=(const Size value)
{
    this->reset(value);
}

bool NXSize::operator==(const Size value)
{
    return this->currentValue == value;
}

}
