//
//  UIControlGestureRecognizer.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 05.04.2023.
//

#pragma once

#include <UIControl.h>
#include <UIGestureRecognizer.h>
#include <UIControlGestureRecognizer.h>

namespace NXKit {

class UIControlGestureRecognizer: public UIGestureRecognizer {
public:
    explicit UIControlGestureRecognizer(const std::weak_ptr<UIControl>& control);

    void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;

    void pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;
    void pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;
    void pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;
    void pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;

private:
    std::weak_ptr<UIControl> control;
    std::shared_ptr<UITouch> _touchToTrack;

    friend class UIControl;
};

}
