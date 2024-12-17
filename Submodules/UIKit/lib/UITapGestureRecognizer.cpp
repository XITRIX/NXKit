#include <UITapGestureRecognizer.h>
#include <UITouch.h>
#include <UIPress.h>

namespace NXKit {

#define THRESHOLD 10

void UITapGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesBegan(touches, event);
    if (!trackingTouch) {
        trackingTouch = touches[0];
        startPoint = trackingTouch->locationIn(this->view().lock());
    }
}

void UITapGestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesMoved(touches, event);
    if (trackingTouch == touches[0]) {
        NXPoint diff = startPoint - trackingTouch->locationIn(this->view().lock());
        if (abs(diff.x) >= THRESHOLD || abs(diff.y) >= THRESHOLD) {
            setState(UIGestureRecognizerState::failed);
            trackingTouch = nullptr;
        }
    }
}

void UITapGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesEnded(touches, event);
    if (trackingTouch == touches[0]) {
        setState(UIGestureRecognizerState::ended);
        trackingTouch = nullptr;
    }
}

void UITapGestureRecognizer::pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    UIGestureRecognizer::pressesBegan(presses, event);
    if (presses.empty()) return;

    if (presses[0]->key()->type() == UIPressType::select) {
        setState(UIGestureRecognizerState::began);
    }
}

void UITapGestureRecognizer::pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    UIGestureRecognizer::pressesEnded(presses, event);

    if (presses.empty()) return;

    if (presses[0]->key()->type() == UIPressType::select) {
        setState(UIGestureRecognizerState::ended);
    }
}

}
