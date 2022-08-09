//
//  UIViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#pragma once

#include "UIResponder.hpp"
#include "UIView.hpp"

namespace NXKit {

class UIViewController: public UIResponder {
public:
    UIView* getView();
    void setView(UIView* view);

    virtual void loadView();
    void loadViewIfNeeded();

    virtual void viewDidLoad() {}
    virtual void viewWillAppear(bool animated) {}
    virtual void viewDidAppear(bool animated) {}
    virtual void viewWillDisappear(bool animated) {}
    virtual void viewDidDisappear(bool animated) {}

    virtual void viewWillLayoutSubviews() {}
    virtual void viewDidLayoutSubviews() {}

    UIResponder* getNext() override;
private:
    UIView* view = nullptr;
};

}
