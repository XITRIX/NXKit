#include <CASpringAnimation.h>

namespace NXKit {

CASpringAnimation::CASpringAnimation(CASpringAnimation* animation):
    CABasicAnimation(animation),
    damping(animation->damping),
    initialSpringVelocity(animation->initialSpringVelocity)
{ }

CASpringAnimation::CASpringAnimation(std::shared_ptr<CASpringAnimationPrototype> prototype,
                                     std::string keyPath,
                                     AnimatableProperty fromValue):
    CABasicAnimation(prototype, keyPath, fromValue, new_shared<CAMediaTimingFunction>(CAMediaTimingFunction::easeOutElastic)),
    damping(prototype->damping),
    initialSpringVelocity(prototype->initialSpringVelocity)
{ }

}
