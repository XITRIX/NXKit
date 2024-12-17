#include <UIEvent.h>
#include <UITouch.h>

namespace NXKit {

std::vector<std::shared_ptr<UIEvent>> UIEvent::activeEvents;

UIEvent::UIEvent() {};
UIEvent::UIEvent(std::shared_ptr<UITouch> touch) {
    _allTouches.insert(touch);
}

}

