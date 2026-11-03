#include <CADisplayLink.h>
#include <DispatchQueue.h>

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

namespace NXKit {
class DispatchQueueTestHarness {
public:
    static void pumpMain() { DispatchQueue::main()->performAll(); }
};
}

int main() {
    const auto mainThread = std::this_thread::get_id();
    std::atomic<int> frames = 0;
    std::atomic<bool> wrongThread = false;
    NXKit::CADisplayLink link([&] {
        ++frames;
        if (std::this_thread::get_id() != mainThread) wrongThread = true;
    });
    std::promise<void> drained;
    NXKit::DispatchQueue::global()->async([&] { drained.set_value(); });
    if (drained.get_future().wait_for(std::chrono::seconds(2)) != std::future_status::ready) {
        std::cerr << "FAIL: the background queue did not drain\n";
        return 1;
    }
    if (frames != 0 || wrongThread) {
        std::cerr << "FAIL: a background queue invoked UI display links\n";
        return 1;
    }
    NXKit::DispatchQueueTestHarness::pumpMain();
    if (frames != 1 || wrongThread) {
        std::cerr << "FAIL: the main queue did not invoke the display link on the UI thread\n";
        return 1;
    }
    link.invalidate();
    NXKit::DispatchQueueTestHarness::pumpMain();
    if (frames != 1) {
        std::cerr << "FAIL: an invalidated display link continued running\n";
        return 1;
    }
    int removedCalls = 0, addedCalls = 0;
    bool replaced = false;
    std::unique_ptr<NXKit::CADisplayLink> removed, added;
    NXKit::CADisplayLink mutator([&] {
        if (replaced) return;
        replaced = true;
        removed.reset();
        added = std::make_unique<NXKit::CADisplayLink>([&] { ++addedCalls; });
    });
    removed = std::make_unique<NXKit::CADisplayLink>([&] { ++removedCalls; });
    NXKit::DispatchQueueTestHarness::pumpMain();
    if (removedCalls || addedCalls) {
        std::cerr << "FAIL: removed/new links ran from a stale tick snapshot\n";
        return 1;
    }
    NXKit::DispatchQueueTestHarness::pumpMain();
    if (removedCalls || addedCalls != 1) {
        std::cerr << "FAIL: replacement link did not begin on the next tick\n";
        return 1;
    }
    mutator.invalidate();
    added.reset();

    int observed = 0;
    NXKit::CADisplayLink stateful([count = 0, &observed]() mutable {
        observed = ++count;
    });
    for (int i = 0; i < 3; ++i) NXKit::DispatchQueueTestHarness::pumpMain();
    stateful.invalidate();
    if (observed != 3) {
        std::cerr << "FAIL: a display link lost its mutable callback state between ticks\n";
        return 1;
    }

    auto capture = std::make_shared<int>(73);
    std::weak_ptr<int> weakCapture = capture;
    std::unique_ptr<NXKit::CADisplayLink> selfRemoving;
    bool completed = false;
    selfRemoving = std::make_unique<NXKit::CADisplayLink>([&, capture] {
        selfRemoving.reset();
        completed = *capture == 73;
    });
    capture.reset();
    NXKit::DispatchQueueTestHarness::pumpMain();
    if (!completed || !weakCapture.expired()) {
        std::cerr << "FAIL: self-removal destroyed the active callback or retained its captures\n";
        return 1;
    }
    std::cout << "Display-link main-queue confinement and callback-lifetime tests passed\n";
}
