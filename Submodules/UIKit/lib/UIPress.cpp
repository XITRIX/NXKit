#include <UIPress.h>
#include <UIWindow.h>
#include <UIViewController.h>

namespace NXKit {

UIPress::UIPress(Timer timestamp):
    _timestamp(timestamp)
{ }

void UIPress::setForWindow(std::shared_ptr<UIWindow> window) {
    _window = window;

    if (!window || !window->rootViewController()) {
        _responder = window;
        return;
    }

//    auto weakFocus = window->focusSystem()->focusedItem();
//    if (!weakFocus.expired()) {
//        auto focus = std::dynamic_pointer_cast<UIResponder>(weakFocus.lock());
//        if (focus) {
//            _responder = focus;
//            return;
//        }
//    }

    auto vc = window->rootViewController();
    while (!vc->children().empty()) {
        vc = vc->children().front();
    }

    _responder = vc->view();
}

void UIPress::runPressActionOnRecognizerHierachy(std::function<void(std::shared_ptr<UIGestureRecognizer>)> action) {
    for (auto recognizer: _gestureRecognizers) {
        if (_hasBeenCancelledByAGestureRecognizer) return;
        if (recognizer.expired() || !recognizer.lock()->isEnabled()) continue;
        action(recognizer.lock());
    }
}

}

