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
    void touchesBegan(std::vector<UITouch*> touches, UIEvent* event) override;
    void touchesMoved(std::vector<UITouch*> touches, UIEvent* event) override;
    void touchesEnded(std::vector<UITouch*> touches, UIEvent* event) override;

private:
    UITouch* trackingTouch = nullptr;
    Point startPoint;
};

}