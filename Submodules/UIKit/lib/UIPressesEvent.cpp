#include <UIPressesEvent.h>

namespace NXKit {

std::vector<std::shared_ptr<UIPressesEvent>> UIPressesEvent::activePressesEvents;

UIPressesEvent::UIPressesEvent(std::shared_ptr<UIPress> press) {
    _allPresses.insert(press);
}

std::set<std::shared_ptr<UIPress>> UIPressesEvent::pressesFor(std::shared_ptr<UIGestureRecognizer> gesture) {
    return {};
}

}

