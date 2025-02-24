#pragma once

#include <UIViewAnimationGroup.h>
#include <tools/SharedBase.hpp>
#include <memory>
#include <any>

namespace NXKit {
    
using AnimatableProperty = std::any;

class CABasicAnimation;
class CABasicAnimationPrototype: public enable_shared_from_this<CABasicAnimationPrototype> {
public:
    const double duration;
    const double delay;
    const std::shared_ptr<UIViewAnimationGroup> animationGroup;

    CABasicAnimationPrototype(double duration, double delay, std::shared_ptr<UIViewAnimationGroup> animationGroup);

    virtual std::shared_ptr<CABasicAnimation> createAnimation(std::string keyPath, AnimatableProperty fromValue);
};

}
