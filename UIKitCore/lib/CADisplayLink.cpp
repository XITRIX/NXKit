#include <CADisplayLink.h>
#include <utility>
#include <algorithm>

namespace NXKit {
    std::vector<CADisplayLink*> CADisplayLink::activeLinks;

    CADisplayLink::CADisplayLink(std::function<void()> func): func(std::move(func)) {
        activeLinks.push_back(this);
    }

    CADisplayLink::~CADisplayLink() {
        invalidate();
    }

    void CADisplayLink::invalidate() {
        if (!isRunning) return;

        isRunning = false;
        activeLinks.erase(std::remove(activeLinks.begin(), activeLinks.end(), this), activeLinks.end());
    }
}
