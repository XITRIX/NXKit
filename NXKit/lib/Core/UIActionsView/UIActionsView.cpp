//
//  UIActionsView.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 06.09.2022.
//

#include <Core/UIActionsView/UIActionsView.hpp>
#include <Core/Application/Application.hpp>

namespace NXKit {

UIActionsView::UIActionsView() {
    focusChangeToken = Application::shared()->getFocusDidChangeEvent()->subscribe([this](UIView* focusView) {
        this->refreshActionsView(focusView);
    });
    refreshActionsView(Application::shared()->getFocus());
}

UIActionsView::~UIActionsView() {
    Application::shared()->getFocusDidChangeEvent()->unsubscribe(focusChangeToken);
}

void UIActionsView::refreshActionsView(UIView* view) {
    
}

}
