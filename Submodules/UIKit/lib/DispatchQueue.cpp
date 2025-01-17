//
//  DispatchQueue.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 10.03.2023.
//

#include <DispatchQueue.h>
#include <thread>
 #include <tools/SharedBase.hpp>
 #include <CADisplayLink.h>

namespace NXKit {

std::shared_ptr<DispatchQueue> DispatchQueue::_main = nullptr;
std::shared_ptr<DispatchQueue> DispatchQueue::_global = nullptr;

std::shared_ptr<DispatchQueue> DispatchQueue::main() {
    if (!_main) _main = new_shared<DispatchQueue>("main");
    return _main;
}

std::shared_ptr<DispatchQueue> DispatchQueue::global() {
    if (!_global) _global = new_shared<DispatchQueue>("global");
    return _global;
}

DispatchQueue::DispatchQueue(const std::string& tag): _tag(tag) {
    if (tag != "main") {
        pthread_create(&_task_loop_thread, nullptr, task_loop, this);
    }
}

DispatchQueue::~DispatchQueue() {
    _task_loop_active = false;
    if (_tag != "main") {
        pthread_join(_task_loop_thread, nullptr);
    }
}

void DispatchQueue::async(const std::function<void()>& task) {
    std::lock_guard<std::mutex> guard(_m_async_mutex);
    _queue.push({ task });
}

void DispatchQueue::asyncAfter(double seconds, const std::function<void()>& task) {
    std::lock_guard<std::mutex> guard(_m_async_mutex);
    _queue.push({ task, Timer(seconds * 1000) });
}

void DispatchQueue::performAll() {
    std::vector<CADisplayLink*> linksCopy = CADisplayLink::activeLinks;
    for (auto link: linksCopy) {
        if (link->isRunning)
            link->func();
    }

    std::queue<DispatchQueueTask> copy;
    {
        std::lock_guard<std::mutex> guard(_m_async_mutex);
        std::swap( _queue, copy );
    }

    Timer now;
    while (!copy.empty()) {
        auto task = copy.front();
        copy.pop();

        // If task is delayed, put it back into queue
        if (now - task.delay < 0) {
            _queue.push(task);
        } else {
            task.func();
        }
    }
}

void* DispatchQueue::task_loop(void* a) {
    auto self = (DispatchQueue*) a;
    while (self->_task_loop_active) {
        self->performAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return nullptr;
}

}
