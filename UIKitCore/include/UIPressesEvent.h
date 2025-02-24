#pragma once

#include <UIEvent.h>

namespace NXKit {

class UIPress;
class UIGestureRecognizer;
class UIPressesEvent: public UIEvent {
public:
    std::set<std::shared_ptr<UIPress>> pressesFor(std::shared_ptr<UIGestureRecognizer> gesture);
    std::set<std::shared_ptr<UIPress>> allPresses() { return _allPresses; }
private:
    explicit UIPressesEvent(std::shared_ptr<UIPress> press);

    static std::vector<std::shared_ptr<UIPressesEvent>> activePressesEvents;
    std::set<std::shared_ptr<UIPress>> _allPresses;

    friend class UIApplication;
};

}

