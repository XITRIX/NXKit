#pragma once

#include <UIView.h>

namespace NXKit {

class UIViewController: public UIResponder, public UITraitEnvironment, public UIFocusEnvironment, public enable_shared_from_this<UIViewController> {
public:
    std::map<std::string, std::shared_ptr<UIView>> idStorage;

    void setView(std::shared_ptr<UIView> view);
    std::shared_ptr<UIView> view();
    bool viewIsLoaded();
    void loadViewIfNeeded();

    std::shared_ptr<UIResponder> next() override;

    // Most of these methods are designed to be overriden in `UIViewController` subclasses
    virtual void loadView();
    virtual void viewDidLoad() {}
    virtual void viewWillAppear(bool animated);
    virtual void viewDidAppear(bool animated);
    virtual void viewWillDisappear(bool animated);
    virtual void viewDidDisappear(bool animated);

    virtual void viewWillLayoutSubviews() {}
    virtual void viewDidLayoutSubviews() {}

    std::vector<std::shared_ptr<UIViewController>> children() { return _children; }
    std::weak_ptr<UIViewController> parent() { return _parent; }

    void addChild(const std::shared_ptr<UIViewController>& child);
    virtual void willMoveToParent(const std::shared_ptr<UIViewController>& parent);
    virtual void didMoveToParent(std::shared_ptr<UIViewController> parent);
    void removeFromParent();

    UIEdgeInsets additionalSafeAreaInsets() { return _additionalSafeAreaInsets; }
    void setAdditionalSafeAreaInsets(UIEdgeInsets additionalSafeAreaInsets);

    UIEdgeInsets systemMinimumLayoutMargins() { return _systemMinimumLayoutMargins; }

    bool viewRespectsSystemMinimumLayoutMargins() { return _viewRespectsSystemMinimumLayoutMargins; }
    void setViewRespectsSystemMinimumLayoutMargins(bool viewRespectsSystemMinimumLayoutMargins);

    void present(const std::shared_ptr<UIViewController>& otherViewController, bool animated, const std::function<void()>& completion = [](){});
    void dismiss(bool animated, const std::function<void()>& completion = [](){});


    void traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) override;

    // Focus
    virtual std::shared_ptr<UIFocusEnvironment> parentFocusEnvironment() override;

protected:
    virtual void makeViewAppear(bool animated, std::shared_ptr<UIViewController> presentingViewController, std::function<void()> completion = [](){});
    virtual void makeViewDisappear(bool animated, std::function<void(bool)> completion);

private:
    std::shared_ptr<UIView> _view;
    std::weak_ptr<UIViewController> _parent;
    std::vector<std::shared_ptr<UIViewController>> _children;
    UIEdgeInsets _additionalSafeAreaInsets;
    UIEdgeInsets _systemMinimumLayoutMargins = UIEdgeInsets(0, 16, 0, 16);
    bool _viewRespectsSystemMinimumLayoutMargins = true;
    float _animationTime = 0.5;

    std::shared_ptr<UIViewController> _presentedViewController;
    std::weak_ptr<UIViewController> _presentingViewController;

    std::shared_ptr<UIViewController> rootVC();
};

}
