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
    UIControlGestureRecognizer(UIControl* control);

    virtual void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    virtual void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    virtual void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    virtual void touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;

    virtual void pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;
    virtual void pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;
    virtual void pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;
    virtual void pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;

private:
    UIControl* control;

    friend class UIControl;
};

}
