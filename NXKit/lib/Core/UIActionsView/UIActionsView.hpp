//
//  UIActionsView.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 06.09.2022.
//

#pragma once

#include <Core/UIStackView/UIStackView.hpp>
#include <Core/Utils/NotificationEvent.hpp>

namespace NXKit {

class UIActionView;
class UIActionsView: public UIStackView {
public:
    UIActionsView();
    ~UIActionsView();

    std::shared_ptr<UIViewController> inController = nullptr;

    void refreshActionsView(std::shared_ptr<UIView> view);
private:
    NotificationEvent<std::shared_ptr<UIView>>::Subscription focusChangeToken;
    std::vector<std::shared_ptr<UIView>> actionViewsQueue;

    std::shared_ptr<UIActionView> dequeueActionView();
};

}
