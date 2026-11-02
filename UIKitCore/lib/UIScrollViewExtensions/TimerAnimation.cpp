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
    advanceTo((Timer() - firstFrameTimestamp) / 1000);
}

void TimerAnimation::advanceTo(double elapsed) {
    if (!_running) return;
    // A scroll callback can cancel this timer or replace it with edge return.
    // Keep the current invocation alive until it has left its own members.
    const auto lifetime=weak_from_this().lock();

    // Touch-driven scrolling already gets a 120 Hz boost from active input.
    // Deceleration runs after the touch ends, so request it explicitly here.
    CALayer::requestFramerate(120);

    if (elapsed >= duration) {
        _running = false;
        displayLink.invalidate();
        animations(1, duration);
        completion(true);
    } else {
        animations(elapsed / duration, elapsed);
    }
}
}
