//
// Created by Daniil Vinogradov on 12/01/2024.
//

#include <UIScrollViewExtensions/TimerAnimation.h>
#include <CALayer.h>
#include <utility>

namespace NXKit {

TimerAnimation::TimerAnimation(double duration, std::function<void(float, double)> animations, std::function<void(bool)> completion):
        displayLink(CADisplayLink([this]() {
            handleFrame();
        })),
        duration(duration),
        animations(std::move(animations)),
        completion(std::move(completion))
{}

TimerAnimation::~TimerAnimation() {
    invalidate();
}

void TimerAnimation::invalidate() {
    if (!_running) return;
    _running = false;
    displayLink.invalidate();
}

void TimerAnimation::handleFrame() {
    if (!_running) return;

    // Touch-driven scrolling already gets a 120 Hz boost from active input.
    // Deceleration runs after the touch ends, so request it explicitly here.
    CALayer::requestFramerate(120);

    auto elapsed = (Timer() - firstFrameTimestamp) / 1000;
    if (elapsed >= duration) {
        animations(1, duration);
        _running = false;
        completion(true);
        displayLink.invalidate();
    } else {
        animations(elapsed / duration, elapsed);
    }
}
}
