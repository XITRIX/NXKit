//
//  UIControlGestureRecognizer.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 05.04.2023.
//

#pragma once

#include <UIControl.h>
#include <UIGestureRecognizer.h>

namespace NXKit {

class UIControlGestureRecognizer: public UIGestureRecognizer {
public:
    explicit UIControlGestureRecognizer(const std::weak_ptr<UIControl>& control);
    bool canBePreventedBy(const std::shared_ptr<UIGestureRecognizer>& other) const override;

    void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;

    void pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;
    void pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;
    void pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;
    void pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;

protected:
    void reset() override;

private:
    std::weak_ptr<UIControl> control;
    std::shared_ptr<UITouch> _touchToTrack;
    NXPoint _contactOrigin, _peakTravel, _completedTravel;
    std::weak_ptr<UITouch> _completedTouch;
    bool _capturesDrag = false, _inputEnded = false;
    std::uint64_t _contactGeneration = 0;
    void claimContact();
    void rejectContact(const std::shared_ptr<UITouch>& touch);

    friend class UIControl;
};

}
