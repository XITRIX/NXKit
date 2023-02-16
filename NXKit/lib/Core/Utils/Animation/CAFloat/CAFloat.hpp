//
//  CAFloat.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 27.08.2022.
//

#pragma once

#include <tweeny/tweeny.h>
#include <Core/Utils/Animation/Core/Time.hpp>

namespace NXKit
{

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
class CAFloat : public FiniteTicking
{
  public:
    /**
     * Creates an animatable with the given initial value.
     */
    CAFloat(float value = 0.0f);

    /**
     * Returns the current animatable value.
     */
    float getValue();

    /**
     * Stops and resets the animation, going back to the given initial value.
     * All steps are removed.
     * If an animation was already ongoing for that animatable, its end callback
     * will be called.
     */
    void reset(float initialValue);

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
    void addStep(float targetValue, int32_t duration, EasingFunction easing = EasingFunction::linear);

    /**
     * Returns the progress of the animation between 0.0f and 1.0f.
     */
    float getProgress();

    operator float() const;
    operator float();
    void operator=(const float value);
    bool operator==(const float value);

  protected:
    bool onUpdate(Time delta) override;

    void onReset() override;
    void onRewind() override;

  private:
    float currentValue = 0.0f;
    tweeny::tween<float> tween;
};

} // namespace brls
