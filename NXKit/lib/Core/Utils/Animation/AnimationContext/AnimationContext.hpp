//
//  AnimationContext.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 27.08.2022.
//

#pragma once

#include <tweeny/tweeny.h>
#include <Core/Utils/Animation/Core/Time.hpp>
#include <deque>

namespace NXKit {

using EasingFunction = tweeny::easing::enumerated;

class AnimationContext: public FiniteTicking {
public:
    /**
     * Creates an animatable with the given initial value.
     */
    AnimationContext();

    /**
     * Returns the current animatable value.
     */
    std::deque<float> getValue();

    /**
     * Stops and resets the animation, going back to the given initial value.
     * All steps are removed.
     * If an animation was already ongoing for that animatable, its end callback
     * will be called.
     */
    void reset(std::deque<float> initialValue);

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
    void addStep(std::deque<float> targetValue, int32_t duration, EasingFunction easing = EasingFunction::linear);

    /**
     * Returns the progress of the animation between 0.0f and 1.0f.
     */
    float getProgress();

//    operator float() const;
//    operator float();
//    void operator=(const float value);
//    bool operator==(const float value);

protected:
    bool onUpdate(Time delta) override;

    void onReset() override;
    void onRewind() override;

private:
    std::deque<float> currentValues;
    std::vector<tweeny::tween<float>> tweens;
};

}
