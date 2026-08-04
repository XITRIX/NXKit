#include <UIPress.h>
#include <UIWindow.h>
#include <UIViewController.h>

namespace NXKit {

UIPress::UIPress(Timer timestamp):
    _timestamp(timestamp)
{ }

void UIPress::setForWindow(const std::shared_ptr<UIWindow>& window) {
    _window = window;

    if (!window || !window->rootViewController()) {
        _responder = window;
        return;
    }

    const auto focusRoot = window->focusRootView();
    if (const auto focusedItem = window->focusSystem()->focusedItem().lock()) {
        if (const auto focusedView = std::dynamic_pointer_cast<UIView>(focusedItem);
            focusedView && focusRoot && focusedView->isDescendantOf(focusRoot)) {
            const auto focusedResponder =
                std::dynamic_pointer_cast<UIResponder>(focusedView);
            _responder = focusedResponder;
            return;
        }
    }

    // A container's first child is not necessarily its visible child. In
    // particular, tab and navigation controllers keep inactive controllers
    // contained, so walking children().front() can select a detached view and
    // terminate the responder chain before the container sees the press.
    // Start at the active presentation root when no item is focused. Its next
    // responder is the presented controller, so modal fallbacks cannot be
    // bypassed by an empty focus hierarchy.
    _responder = focusRoot;
}

void UIPress::runPressActionOnRecognizerHierachy(const std::function<void(std::shared_ptr<UIGestureRecognizer>)>& action) {
    for (const auto& recognizer: _gestureRecognizers) {
        if (_hasBeenCancelledByAGestureRecognizer) return;
        if (recognizer.expired() || !recognizer.lock()->isEnabled()) continue;
        action(recognizer.lock());
    }
}

}
