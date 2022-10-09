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
    std::shared_ptr<UIImage> image = nullptr;
};

class UIViewController: public UIResponder, public UITraitEnvironment, public enable_shared_from_base<UIViewController> {
public:
    UIViewController();
    virtual ~UIViewController();

    std::shared_ptr<UIView> getView();
    void setView(std::shared_ptr<UIView> view);

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

    std::vector<std::shared_ptr<UIViewController>> getChildren() { return children; }
    std::shared_ptr<UIViewController> getParent() { return parent.lock(); }

    void addChild(std::shared_ptr<UIViewController> child);
    void willMoveToParent(std::shared_ptr<UIViewController> parent);
    void didMoveToParent(std::shared_ptr<UIViewController> parent);
    void removeFromParent();

    UIEdgeInsets getAdditionalSafeAreaInsets() { return additionalSafeAreaInsets; }
    void setAdditionalSafeAreaInsets(UIEdgeInsets insets);

    std::shared_ptr<UIViewController> getPresentedViewController();
    std::shared_ptr<UIViewController> getPresentingViewController();
    void present(std::shared_ptr<UIViewController> controller, bool animated, std::function<void()> completion = [](){});
    void dismiss(bool animated, std::function<void()> completion = [](){});

    virtual void show(std::shared_ptr<UIViewController> controller, void* sender = nullptr);

    std::shared_ptr<UIResponder> getNext() override;

    UITraitCollection getTraitCollection() override;
    UIUserInterfaceStyle overrideUserInterfaceStyle = UIUserInterfaceStyle::unspecified;

    std::string getTitle() { return navigationItem.title; }
    void setTitle(std::string title);

    std::shared_ptr<UIImage> getImage() { return navigationItem.image; }
    void setImage(std::shared_ptr<UIImage> image);

    UINavigationItem getNavigationItem() { return navigationItem; }
    virtual void childNavigationItemDidChange(std::shared_ptr<UIViewController> controller) {}

protected:
    virtual void makeViewAppear(bool animated, std::shared_ptr<UIViewController> presentingViewController, std::function<void()> completion = [](){});
    virtual void makeViewDisappear(bool animated, std::function<void(bool)> completion);

private:
    std::string title;
    UIEdgeInsets additionalSafeAreaInsets;
    std::vector<std::shared_ptr<UIViewController>> children;
    std::weak_ptr<UIViewController> parent;
    std::shared_ptr<UIView> view = nullptr;
    UINavigationItem navigationItem;

    void setNavigationItem(UINavigationItem item);

    bool dismissing = false;

    std::shared_ptr<UIViewController> presentedViewController = nullptr;
    std::shared_ptr<UIViewController> presentingViewController = nullptr;

    void setPresentedViewController(std::shared_ptr<UIViewController> presentedViewController);
    void setPresentingViewController(std::shared_ptr<UIViewController> presentingViewController);
};

}
