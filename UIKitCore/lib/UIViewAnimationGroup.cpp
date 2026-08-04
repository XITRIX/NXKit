#include <UIViewAnimationGroup.h>

#include <utility>

namespace NXKit {

UIViewAnimationGroup::UIViewAnimationGroup(UIViewAnimationOptions options, std::optional<std::function<void(bool)>> completion):
    options(options), completion(std::move(completion))
{ }

void UIViewAnimationGroup::animationDidStop(bool finished) {
    _allAnimationsFinished = _allAnimationsFinished && finished;
    queuedAnimations -= 1;
    if (queuedAnimations == 0) {
        auto finalCompletion = std::move(completion);
        completion.reset();
        if (finalCompletion.has_value()) {
            finalCompletion.value()(_allAnimationsFinished);
        }
    }
}

}
