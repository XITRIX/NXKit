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
    std::cout << "Display-link main-queue confinement tests passed\n";
}
