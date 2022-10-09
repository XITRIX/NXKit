//
//  UITapGestureRecognizer.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 20.08.2022.
//

#pragma once

#include <Core/UIGestureRecognizer/UIGestureRecognizer.hpp>
#include <Core/UITouch/UITouch.hpp>
#include <functional>

namespace NXKit {

class UITapGestureRecognizer: public UIGestureRecognizer {
public:
    void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;
    void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) override;

private:
    std::shared_ptr<UITouch> trackingTouch;
    Point startPoint;
};

}
