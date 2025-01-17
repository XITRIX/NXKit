#pragma once

#include <CABasicAnimation.h>
#include <CASpringAnimationPrototype.h>

namespace NXKit {

class CASpringAnimation: public CABasicAnimation {
public:
    std::optional<double> damping;
    std::optional<double> initialSpringVelocity;

    CASpringAnimation(CASpringAnimation* animation);
    CASpringAnimation(const std::shared_ptr<CASpringAnimationPrototype>& prototype,
                      std::string keyPath,
                      AnimatableProperty fromValue);
};

}
