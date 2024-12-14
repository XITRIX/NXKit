#include <CASpringAnimationPrototype.h>
#include <CASpringAnimation.h>

namespace NXKit {

CASpringAnimationPrototype::CASpringAnimationPrototype(double duration,
                                                       double delay,
                                                       double damping,
                                                       double initialSpringVelocity,
                                                       std::shared_ptr<UIViewAnimationGroup> animationGroup) :
    CABasicAnimationPrototype(duration, delay, animationGroup),
    damping(damping),
    initialSpringVelocity(initialSpringVelocity)
{ }

std::shared_ptr<CABasicAnimation> CASpringAnimationPrototype::createAnimation(std::string keyPath, AnimatableProperty fromValue) {
    auto self = std::static_pointer_cast<CASpringAnimationPrototype>(shared_from_this());
    return new_shared<CASpringAnimation>(self, keyPath, fromValue);
}

}
