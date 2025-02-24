#pragma once

#include <CABasicAnimationPrototype.h>

namespace NXKit {

class CASpringAnimation;
class CASpringAnimationPrototype: public CABasicAnimationPrototype {
public:
    const double damping;
    const double initialSpringVelocity;

    CASpringAnimationPrototype(double duration,
                               double delay,
                               double damping,
                               double initialSpringVelocity,
                               std::shared_ptr<UIViewAnimationGroup> animationGroup);

    std::shared_ptr<CABasicAnimation> createAnimation(std::string keyPath, AnimatableProperty fromValue) override;
    
};

}
