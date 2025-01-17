#include <CASpringAnimation.h>

#include <utility>

namespace NXKit {

CASpringAnimation::CASpringAnimation(CASpringAnimation* animation):
    CABasicAnimation(animation),
    damping(animation->damping),
    initialSpringVelocity(animation->initialSpringVelocity)
{ }

CASpringAnimation::CASpringAnimation(const std::shared_ptr<CASpringAnimationPrototype>& prototype,
                                     std::string keyPath,
                                     AnimatableProperty fromValue):
    CABasicAnimation(prototype, std::move(keyPath), std::move(fromValue), new_shared<CAMediaTimingFunction>(CAMediaTimingFunction::easeOutElastic)),
    damping(prototype->damping),
    initialSpringVelocity(prototype->initialSpringVelocity)
{ }

}
