//
// Created by Даниил Виноградов on 24.02.2025.
//

#pragma once
#include <UIControl.h>

namespace NXKit {

class NXControl: public UIControl {
    void didUpdateFocusIn(UIFocusUpdateContext context, UIFocusAnimationCoordinator* coordinator) override;

    void willGainFocus() override;

    void willLoseFocus() override;

    void focusFailedToMove(UIFocusHeading heading) override;

    void willChangeFocusHighlight(bool highlighted) override;

    void willChangeHighlight(bool highlighted) override;
};



}