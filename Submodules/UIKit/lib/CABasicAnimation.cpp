#include <CABasicAnimation.h>
#include <cmath>

using namespace NXKit;

CABasicAnimation::CABasicAnimation(std::string keyPath) {
    timingFunction = new_shared<CAMediaTimingFunction>(kCAMediaTimingFunctionDefault);
    this->keyPath = keyPath;
}

CABasicAnimation::CABasicAnimation(std::shared_ptr<CABasicAnimationPrototype> prototype, std::string keyPath, AnimatableProperty fromValue, std::shared_ptr<CAMediaTimingFunction> timingFunction):
    CABasicAnimation(keyPath)
{
    delay = prototype->delay;
    duration = prototype->duration;
    animationGroup = prototype->animationGroup;

    this->fromValue = fromValue;
    this->timingFunction = timingFunction;
}

CABasicAnimation::CABasicAnimation(CABasicAnimation* animation) {
    keyPath = animation->keyPath;
    duration = animation->duration;
    delay = animation->delay;
    creationTime = animation->creationTime;
    fillMode = animation->fillMode;
    fromValue = animation->fromValue;
    toValue = animation->toValue;
    animationGroup = animation->animationGroup;
    isRemovedOnCompletion = animation->isRemovedOnCompletion;
    timingFunction = animation->timingFunction;
}

bool CABasicAnimation::wasCreatedInUIAnimateBlock() {
    return animationGroup != nullptr;
}

float CABasicAnimation::progressFor(Timer currentTime) {
    auto elapsedTimeMinusDelayInMs = float(currentTime - creationTime) - (delay * 1000);

    // prevents a division by zero when animating with duration: 0
    if (duration <= 0) {
        auto animationHasStarted = (elapsedTimeMinusDelayInMs > 0);
        return animationHasStarted ? 1.0f : 0.0f;
    }

    auto durationInMs = duration * 1000;
    auto linearProgress = std::fmax(0, std::fmin(1, elapsedTimeMinusDelayInMs / durationInMs));
    return ease(linearProgress);
}

float CABasicAnimation::ease(float x) {
    if (timingFunction) { return timingFunction->at(x); }
    return x;
}
