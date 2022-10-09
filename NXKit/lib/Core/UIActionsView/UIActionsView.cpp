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

        iconLabel = std::make_shared<UILabel>();
        iconLabel->verticalAlign = VerticalAlign::CENTER;
        iconLabel->getFont()->fontSize = 25.5f;

        label = std::make_shared<UILabel>();
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
    std::shared_ptr<UILabel> label = nullptr;
    std::shared_ptr<UILabel> iconLabel = nullptr;
};

UIActionsView::UIActionsView():
    UIStackView(Axis::HORIZONTAL)
{
    focusChangeToken = Application::shared()->getFocusDidChangeEvent()->subscribe([this](std::shared_ptr<UIView> focusView) {
        this->refreshActionsView(focusView);
    });
    refreshActionsView(Application::shared()->getFocus());
}

UIActionsView::~UIActionsView() {
    Application::shared()->getFocusDidChangeEvent()->unsubscribe(focusChangeToken);
//    for (auto action: actionViewsQueue)
//        delete action;
}

void UIActionsView::refreshActionsView(std::shared_ptr<UIView> view) {
    auto subviews = getSubviews();
    for (auto subview: subviews) {
        subview->removeFromSuperview();
        actionViewsQueue.push_back(subview);
    }

    if (inController) {
        view = inController->getView()->getDefaultFocus();
    }

    std::map<ControllerButton, UIAction> actionsMap;
    std::shared_ptr<UIResponder> responder = view;
    while (responder) {
        for (auto actions: responder->getActions()) {
            if (!actionsMap.count(actions.first)) {
                for (auto action: actions.second) {
                    if (action.condition()) {
                        actionsMap[actions.first] = action;
                        break;
                    }
                }
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

std::shared_ptr<UIActionView> UIActionsView::dequeueActionView() {
    if (actionViewsQueue.size() > 0) {
        auto actionView = std::dynamic_pointer_cast<UIActionView>(actionViewsQueue.back());
        actionViewsQueue.pop_back();
        return actionView;
    }
    return std::make_shared<UIActionView>();
}

}
