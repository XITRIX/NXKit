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
        bool isRunning = true;
        std::function<void()> func;
        static std::vector<CADisplayLink*> activeLinks;
        friend class DispatchQueue;
    };

}