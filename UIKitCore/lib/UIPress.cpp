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

    if (const auto focusedItem = window->focusSystem()->focusedItem().lock()) {
        if (const auto focusedResponder =
                std::dynamic_pointer_cast<UIResponder>(focusedItem)) {
            _responder = focusedResponder;
            return;
        }
    }

    // A container's first child is not necessarily its visible child. In
    // particular, tab and navigation controllers keep inactive controllers
    // contained, so walking children().front() can select a detached view and
    // terminate the responder chain before the container sees the press.
    // Starting at the root view is a safe fallback: its next responder is the
    // root controller, which can handle or forward the press to its container.
    _responder = window->rootViewController()->view();
}

void UIPress::runPressActionOnRecognizerHierachy(const std::function<void(std::shared_ptr<UIGestureRecognizer>)>& action) {
    for (const auto& recognizer: _gestureRecognizers) {
        if (_hasBeenCancelledByAGestureRecognizer) return;
        if (recognizer.expired() || !recognizer.lock()->isEnabled()) continue;
        action(recognizer.lock());
    }
}

}
