#pragma once

#include <UIGestureRecognizer.h>
#include <Geometry.h>

namespace NXKit {

class UITapGestureRecognizer: public UIGestureRecognizer {
public:
    void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;

    void pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;
    void pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;

private:
    std::shared_ptr<UITouch> trackingTouch;
    NXPoint startPoint;
};

}
