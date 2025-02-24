#pragma once

#include <UIViewAnimationOptions.h>
#include <optional>
#include <functional>

namespace NXKit {

class UIViewAnimationGroup {
public:
    std::optional<std::function<void(bool)>> completion;
    int queuedAnimations = 0;
    UIViewAnimationOptions options;

    UIViewAnimationGroup(UIViewAnimationOptions options, std::optional<std::function<void(bool)>> completion);

    void animationDidStop(bool finished);
};

}
