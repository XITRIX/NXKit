//
// Created by Daniil Vinogradov on 12/01/2024.
//

#pragma once

#include <CADisplayLink.h>
#include <Timer.h>
#include <memory>

namespace NXKit {

class TimerAnimation: public std::enable_shared_from_this<TimerAnimation> {
public:
    TimerAnimation(double duration, std::function<void(float, double)> animations, std::function<void(bool)> completion = [](bool _){});
    ~TimerAnimation();

    void invalidate();

private:
    friend class UIScrollViewPhysicsTestHarness;
    CADisplayLink displayLink;
    bool _running = true;
    Timer firstFrameTimestamp;
    double duration;
    std::function<void(float, double)> animations;
    std::function<void(bool)> completion;

    void handleFrame();
    void advanceTo(double elapsed);
};

}

