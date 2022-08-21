//
//  UIViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#pragma once

#include <Core/UIResponder/UIResponder.hpp>
#include <Core/UIView/UIView.hpp>

namespace NXKit {

class UIViewController: public UIResponder {
public:
    UIView* getView();
    void setView(UIView* view);

    virtual void loadView();
    void loadViewIfNeeded();
    bool isViewLoaded();

    virtual void viewDidLoad() {}
    virtual void viewWillAppear(bool animated) {}
    virtual void viewDidAppear(bool animated) {}
    virtual void viewWillDisappear(bool animated) {}
    virtual void viewDidDisappear(bool animated) {}

    virtual void viewWillLayoutSubviews() {}
    virtual void viewDidLayoutSubviews() {}

    std::vector<UIViewController*> getChildren() { return children; }
    UIViewController* getParent() { return parent; }

    std::string getTitle() { return title; }
    void setTitle(std::string title);

    void addChild(UIViewController* child);
    void willMoveToParent(UIViewController* parent);
    void didMoveToParent(UIViewController* parent);
    void removeFromParent();

    UIEdgeInsets getAdditionalSafeAreaInsets() { return additionalSafeAreaInsets; }
    void setAdditionalSafeAreaInsets(UIEdgeInsets insets);

    UIResponder* getNext() override;
private:
    std::string title;
    UIEdgeInsets additionalSafeAreaInsets;
    std::vector<UIViewController*> children;
    UIViewController* parent = nullptr;
    UIView* view = nullptr;
};

}
