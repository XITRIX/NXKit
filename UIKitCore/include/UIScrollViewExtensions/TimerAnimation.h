//
// Created by Daniil Vinogradov on 12/01/2024.
//

#pragma once

#include <CADisplayLink.h>
#include <Timer.h>

namespace NXKit {

class TimerAnimation: enable_shared_from_this_pointer_holder {
public:
    TimerAnimation(double duration, std::function<void(float, double)> animations, std::function<void(bool)> completion = [](bool _){});
    ~TimerAnimation();

    void invalidate();

private:
    CADisplayLink displayLink;
    bool _running = true;
    Timer firstFrameTimestamp;
    double duration;
    std::function<void(float, double)> animations;
    std::function<void(bool)> completion;

    void handleFrame();
};

}

