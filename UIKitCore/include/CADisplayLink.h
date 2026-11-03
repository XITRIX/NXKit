#pragma once

#include <vector>
#include <functional>
#include <tools/SharedBase.hpp>

namespace NXKit {

    class CADisplayLink {
    public:
        explicit CADisplayLink(std::function<void()> func);
        ~CADisplayLink();

        void invalidate();
    private:
        struct Invocation {
            bool isRunning = true;
            std::shared_ptr<std::function<void()>> func;
        };
        std::shared_ptr<Invocation> invocation;
        static std::vector<std::shared_ptr<Invocation>> activeLinks;
        friend class DispatchQueue;
    };

}
