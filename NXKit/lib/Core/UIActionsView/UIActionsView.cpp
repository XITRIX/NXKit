//
//  UIActionsView.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 06.09.2022.
//

#include <Core/UIActionsView/UIActionsView.hpp>
#include <Core/Application/Application.hpp>
#include <Core/UIControl/UIControl.hpp>
#include <Core/UILabel/UILabel.hpp>

namespace NXKit {

class UIActionView: public UIControl {
public:
    UIActionView() {
        setAxis(Axis::HORIZONTAL);
        setPadding(4, 16, 4, 16);
        cornerRadius = 6;

        iconLabel = new UILabel();
        iconLabel->verticalAlign = VerticalAlign::CENTER;
        iconLabel->getFont()->fontSize = 25.5f;

        label = new UILabel();
        label->verticalAlign = VerticalAlign::CENTER;
        label->getFont()->fontSize = 21.5f;
        label->setMarginLeft(8);

        addSubview(iconLabel);
        addSubview(label);

        onEvent = [this](UIControlTouchEvent event) {
            if (event == UIControlTouchEvent::touchUpInside) {
                this->action.action();
            }
        };
    }

    void setAction(ControllerButton button, UIAction action) {
        this->button = button;
        this->action = action;

        isUserInteractionEnabled = button != BUTTON_A;
        iconLabel->setText(InputManager::shared()->getButtonIcon(button));
        label->setText(action.name);
        setEnabled(action.enabled);
    }

    void setEnabled(bool enabled) override {
        UIControl::setEnabled(enabled);

        label->textColor = enabled ? UIColor::label : UIColor::gray;
        iconLabel->textColor = enabled ? UIColor::label : UIColor::gray;
    }
    
    bool canBecomeFocused() override { return false; }
private:
    ControllerButton button;
    UIAction action;
    UILabel* label = nullptr;
    UILabel* iconLabel = nullptr;
};

UIActionsView::UIActionsView():
    UIStackView(Axis::HORIZONTAL)
{
    focusChangeToken = Application::shared()->getFocusDidChangeEvent()->subscribe([this](UIView* focusView) {
        this->refreshActionsView(focusView);
    });
    refreshActionsView(Application::shared()->getFocus());
}

UIActionsView::~UIActionsView() {
    Application::shared()->getFocusDidChangeEvent()->unsubscribe(focusChangeToken);
}

void UIActionsView::refreshActionsView(UIView* view) {
    auto subviews = getSubviews();
    for (auto subview: subviews) {
        subview->removeFromSuperview();
        actionViewsQueue.push_back(subview);
    }

    if (inController) {
        view = inController->getView()->getDefaultFocus();
    }

    std::map<ControllerButton, UIAction> actionsMap;
    UIResponder* responder = view;
    while (responder) {
        for (auto action: responder->getActions()) {
            if (!actionsMap.count(action.first)) {
                actionsMap[action.first] = action.second;
            }
        }
        responder = responder->getNext();
    }

    for (auto action: actionsMap) {
        auto item = dequeueActionView();
        item->setAction(action.first, action.second);
        addSubview(item);
    }

    if (!actionsMap.count(BUTTON_A)) {
        auto item = dequeueActionView();
        item->setAction(BUTTON_A, UIAction([](){}, "OK", false));
        addSubview(item);
    }
}

UIActionView* UIActionsView::dequeueActionView() {
    if (actionViewsQueue.size() > 0) {
        UIActionView* actionView = (UIActionView*) actionViewsQueue.back();
        actionViewsQueue.pop_back();
        return actionView;
    }
    return new UIActionView();
}

}