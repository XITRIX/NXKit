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

#pragma once

#include <Core/Geometry/Geometry.hpp>
#include <tweeny/tweeny.h>

#include <Core/Utils/Animation/Core/Time.hpp>

namespace NXKit {

using EasingFunction = tweeny::easing::enumerated;

// An animatable is a float which value can be animated from an initial value to a target value,
// during a given amount of time. An easing function can also be specified.
//
// Declare the animatable and then use reset(initialValue) to reset the animation.
// Add as many steps as you like by calling addStep(targetValue, duration, easing) one or multiple times.
// Then, start and stop the animation with start() and stop().
//
// setEndCallback() and setTickCallback() allow you to execute code as long as the animation runs and / or once when it finishes.
// Use .getValue() to get the current value at any time.
//
// An animatable has overloads for float conversion, comparison (==) and assignment operator (=) to allow
// basic usage as a simple float. Assignment operator is a shortcut to the reset() method.
class NXSize : public FiniteTicking
{
public:
    /**
     * Creates an animatable with the given initial value.
     */
    NXSize(Size value = Size());

    /**
     * Returns the current animatable value.
     */
    Size getValue();
    float width();
    float height();

    /**
     * Stops and resets the animation, going back to the given initial value.
     * All steps are removed.
     * If an animation was already ongoing for that animatable, its end callback
     * will be called.
     */
    void reset(Size initialValue);

    /**
     * Stops and resets the animation. The value will stay where it's at.
     * All steps are removed.
     * If an animation was already ongoing for that animatable, its end callback
     * will be called.
     */
    void reset();

    /**
     * Adds an animation step to the target value, lasting the specified duration in milliseconds.
     *
     * An animation can have multiple steps. Target value can be greater and lower than the previous step (it can go forwards or backwards).
     * Easing function is optional, default is EasingFunction::linear.
     *
     * Duration is int32_t due to internal limitations, so a step cannot last for longer than 2 147 483 647ms.
     * The sum of the duration of all steps cannot exceed 71582min.
     */
    void addStep(Size targetValue, int32_t duration, EasingFunction easing = EasingFunction::linear);

    /**
     * Returns the progress of the animation between 0.0f and 1.0f.
     */
    float getProgress();

    operator Size() const;
    operator Size();
    void operator=(const Size value);
    bool operator==(const Size value);

protected:
    bool onUpdate(Time delta) override;

    void onReset() override;
    void onRewind() override;

private:
    Size currentValue = Size();
    tweeny::tween<float, float> tween;
};

}