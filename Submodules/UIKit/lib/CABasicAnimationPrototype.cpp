#include <CABasicAnimationPrototype.h>
#include <CABasicAnimation.h>

namespace NXKit {

CABasicAnimationPrototype::CABasicAnimationPrototype(double duration, double delay, std::shared_ptr<UIViewAnimationGroup> animationGroup):
    duration(duration), delay(delay), animationGroup(animationGroup)
{ }

std::shared_ptr<CABasicAnimation> CABasicAnimationPrototype::createAnimation(std::string keyPath, AnimatableProperty fromValue) {
    return new_shared<CABasicAnimation>(
        shared_from_this(),
        keyPath,
        fromValue,
        CAMediaTimingFunction::timingFunctionFrom(animationGroup->options)
    );
}

}

