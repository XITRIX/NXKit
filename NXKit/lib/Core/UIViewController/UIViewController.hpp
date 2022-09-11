//
//  UIViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#pragma once

#include <Core/UIResponder/UIResponder.hpp>
#include <Core/UIImage/UIImage.hpp>
#include <Core/UIView/UIView.hpp>

namespace NXKit {

struct UINavigationItem {
    std::string title;
    UIImage* image = nullptr;
};

class UIViewController: public UIResponder, public UITraitEnvironment {
public:
    virtual ~UIViewController();

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

    void addChild(UIViewController* child);
    void willMoveToParent(UIViewController* parent);
    void didMoveToParent(UIViewController* parent);
    void removeFromParent();

    UIEdgeInsets getAdditionalSafeAreaInsets() { return additionalSafeAreaInsets; }
    void setAdditionalSafeAreaInsets(UIEdgeInsets insets);

    UIViewController* getPresentedViewController();
    UIViewController* getPresentingViewController();
    void present(UIViewController* controller, bool animated, std::function<void()> completion = [](){});
    void dismiss(bool animated, std::function<void()> completion = [](){});

    virtual void show(UIViewController* controller, void* sender = nullptr);

    UIResponder* getNext() override;

    UITraitCollection getTraitCollection() override;
    UIUserInterfaceStyle overrideUserInterfaceStyle = UIUserInterfaceStyle::unspecified;

    std::string getTitle() { return navigationItem.title; }
    void setTitle(std::string title);

    UIImage* getImage() { return navigationItem.image; }
    void setImage(UIImage* image);

    UINavigationItem getNavigationItem() { return navigationItem; }
    virtual void childNavigationItemDidChange(UIViewController* controller) {}

protected:
    virtual void makeViewAppear(bool animated, UIViewController* presentingViewController, std::function<void()> completion = [](){});
    virtual void makeViewDisappear(bool animated, std::function<void(bool)> completion);

private:
    std::string title;
    UIEdgeInsets additionalSafeAreaInsets;
    std::vector<UIViewController*> children;
    UIViewController* parent = nullptr;
    UIView* view = nullptr;
    UINavigationItem navigationItem;

    void setNavigationItem(UINavigationItem item);

    bool dismissing = false;

    UIViewController* presentedViewController = nullptr;
    UIViewController* presentingViewController = nullptr;

    void setPresentedViewController(UIViewController* presentedViewController);
    void setPresentingViewController(UIViewController* presentingViewController);
};

}
