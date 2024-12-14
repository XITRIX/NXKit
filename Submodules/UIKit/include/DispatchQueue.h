#pragma once

#include <queue>
#include <string>
#include <functional>
#include <thread>
#include <mutex>

#ifdef main
#undef main
#endif

namespace NXKit {

class DispatchQueue {
public:
    DispatchQueue(const std::string& tag);
    ~DispatchQueue();

    static std::shared_ptr<DispatchQueue> main();
    static std::shared_ptr<DispatchQueue> global();

    std::string tag() const { return _tag; }
    void async(const std::function<void()>& task);

    bool isActive() { return _task_loop_active; }
private:
    static std::shared_ptr<DispatchQueue> _main;
    static std::shared_ptr<DispatchQueue> _global;
    std::queue<std::function<void()>> _queue;
    std::string _tag;

    pthread_t _task_loop_thread = nullptr;
    std::mutex _m_async_mutex;
    volatile bool _task_loop_active = true;
    static void* task_loop(void* a);

    void performAll();

    friend bool applicationRunLoop();
};

}
