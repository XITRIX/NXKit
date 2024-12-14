#pragma once

#include <CALayer.h>

namespace NXKit {

struct CATransaction {
private:
    bool disableActions_ = false;
    float animationDuration_ = CALayer::defaultAnimationDuration;

    static std::vector<CATransaction> transactionStack;

public:
    static void begin();

    static void commit();

    static bool disableActions();

    static void setDisableActions(bool newValue);

    static float animationDuration();

    static void setAnimationDuration(float newValue);
};

}
