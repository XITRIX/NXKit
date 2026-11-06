#include <UITouch.h>
#include <UIWindow.h>
#include <utility>
#include <algorithm>

namespace NXKit {

UITouch::UITouch(unsigned long touchId, NXPoint atPoint, Timer timestamp, unsigned int tapCount):
_touchId(touchId),
_tapCount(std::max(1u,tapCount)),
_initialLocation(atPoint),
_timestamp(timestamp),
_absoluteLocation(atPoint),
_previousAbsoluteLocation(atPoint)
{ }

void UITouch::updateAbsoluteLocation(NXPoint newLocation) {
    _previousAbsoluteLocation = _absoluteLocation;
    _absoluteLocation = newLocation;
}

NXPoint UITouch::locationIn(std::shared_ptr<UIView> view) {
    if (_window.expired()) return _absoluteLocation;
    return _window.lock()->convertToView(_absoluteLocation, std::move(view));
}

NXPoint UITouch::previousLocationIn(std::shared_ptr<UIView> view) {
    if (_window.expired()) return _previousAbsoluteLocation;
    return _window.lock()->convertToView(_previousAbsoluteLocation, std::move(view));
}

void UITouch::runTouchActionOnRecognizerHierachy(const std::function<void(std::shared_ptr<UIGestureRecognizer>)>& action) {
    for (const auto& recognizer: _gestureRecognizers) {
        if (recognizer.expired() || !recognizer.lock()->isEnabled()) continue;
        action(recognizer.lock());
    }
}


}
