#include <UIViewAnimationGroup.h>

#include <utility>

namespace NXKit {

UIViewAnimationGroup::UIViewAnimationGroup(UIViewAnimationOptions options, std::optional<std::function<void(bool)>> completion):
    options(options), completion(std::move(completion))
{ }

void UIViewAnimationGroup::animationDidStop(bool finished) {
    queuedAnimations -= 1;
    if (queuedAnimations == 0) {
        if (completion.has_value())
            completion.value()(finished);
    }
}

}
