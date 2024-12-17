#include <UIResponder.h>
#include <UITouch.h>
#include <utility>

namespace NXKit {

void UIResponder::touchesBegan(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    auto next = this->next();
    if (next) next->touchesBegan(std::move(touches), std::move(event));
}

void UIResponder::touchesMoved(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    auto next = this->next();
    if (next) next->touchesMoved(std::move(touches), std::move(event));
}

void UIResponder::touchesEnded(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    auto next = this->next();
    if (next) next->touchesEnded(std::move(touches), std::move(event));
}

void UIResponder::touchesCancelled(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    auto next = this->next();
    if (next) next->touchesCancelled(std::move(touches), std::move(event));
}

void UIResponder::pressesBegan(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    auto next = this->next();
    if (next) next->pressesBegan(std::move(pressees), std::move(event));
}

void UIResponder::pressesChanged(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    auto next = this->next();
    if (next) next->pressesChanged(std::move(pressees), std::move(event));
}

void UIResponder::pressesEnded(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    auto next = this->next();
    if (next) next->pressesEnded(std::move(pressees), std::move(event));
}

void UIResponder::pressesCancelled(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    auto next = this->next();
    if (next) next->pressesCancelled(std::move(pressees), std::move(event));
}

}

