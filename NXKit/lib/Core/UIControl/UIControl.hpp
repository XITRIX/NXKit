//
//  UIControl.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 01.09.2022.
//

#pragma once

#include <Core/UIStackView/UIStackView.hpp>
#include <Core/UIGestureRecognizer/UIGestureRecognizer.hpp>

namespace NXKit {

enum class UIControlTouchEvent {
    touchDown,
    touchDragInside,
    touchDragOutside,
    touchUpInside,
    touchUpOutside,
    touchCancel
};

class UIControl: public UIStackView {
public:
    std::function<void(UIControlTouchEvent)> onEvent = [](auto){};

    UIControl();
    ~UIControl();
    bool canBecomeFocused() override;
    bool press(ControllerButton button) override;

    virtual bool isEnabled();
    bool isSelected();
    bool isHighlighted();

protected:
    virtual void setEnabled(bool enabled);
    virtual void setSelected(bool selected);
    virtual void setHighlighted(bool highlighted);

private:
    bool enabled = true;
    bool selected = false;
    bool highlighted = false;

    UIColor backgroundStorage;
    NotificationEvent<>::Subscription actionSub;

    // MARK: - Gesture recognizer
    class GestureRecognizer: public UIGestureRecognizer {
    public:
        GestureRecognizer(UIControl* control);
        
        void touchesBegan(std::vector<UITouch*> touches, UIEvent* event) override;
        void touchesMoved(std::vector<UITouch*> touches, UIEvent* event) override;
        void touchesEnded(std::vector<UITouch*> touches, UIEvent* event) override;
        void touchesCancelled(std::vector<UITouch*> touches, UIEvent* event) override;

    private:
        float extraTouchArea = 20;
        UITouch* trackingTouch = nullptr;
        UIControl* control;
    };
};

}

