#include <CADisplayLink.h>
#include <utility>
#include <algorithm>

namespace NXKit {
    std::vector<std::shared_ptr<CADisplayLink::Invocation>> CADisplayLink::activeLinks;

    CADisplayLink::CADisplayLink(std::function<void()> func)
        : invocation(std::make_shared<Invocation>()) {
        invocation->func = std::make_shared<std::function<void()>>(std::move(func));
        activeLinks.push_back(invocation);
    }

    CADisplayLink::~CADisplayLink() {
        invalidate();
    }

    void CADisplayLink::invalidate() {
        if (!invocation->isRunning) return;

        invocation->isRunning = false;
        invocation->func.reset();
        std::erase(activeLinks, invocation);
    }
}
