#pragma once

#include <Timer.h>
#include <set>
#include <memory>
#include <vector>

namespace NXKit {

class UITouch;
class UIEvent {
public:
    std::set<std::shared_ptr<UITouch>> allTouches() { return _allTouches; }
    Timer timestamp() { return _timestamp; }

    UIEvent();
    virtual ~UIEvent() {}
private:
    UIEvent(std::shared_ptr<UITouch> touch);

    static std::vector<std::shared_ptr<UIEvent>> activeEvents;
    std::set<std::shared_ptr<UITouch>> _allTouches;
    Timer _timestamp = Timer();

    friend class UIApplicationMain;
    friend class UIApplication;
    friend class UIView;
};

}

