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

class UIActionsView: public UIStackView {
public:
    UIActionsView();
    ~UIActionsView();

    void refreshActionsView(UIView* view);
private:
    NotificationEvent<UIView*>::Subscription focusChangeToken;
};

}
