#pragma once

#include "UIView.h"
#include <UIFocusSystem.h>

namespace NXKit {

class UIViewController;
class UIWindow: public UIView {
public:
    UIWindow();
    ~UIWindow();

    std::shared_ptr<UIWindow> window() override;
    std::shared_ptr<UIFocusSystem> focusSystem() { return _focusSystem; }

    void setRootViewController(std::shared_ptr<UIViewController> controller);
    std::shared_ptr<UIViewController> rootViewController() { return _rootViewController; }

    void makeKeyAndVisible();
    void sendEvent(const std::shared_ptr<UIEvent>& event);

    void layoutSubviews() override;
    void updateFocus();

    void pressesBegan(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) override;
    void pressesChanged(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) override;
    void pressesEnded(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) override;
    void pressesCancelled(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) override;

    void traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) override;
private:
    std::shared_ptr<UIViewController> _rootViewController;
    std::vector<std::shared_ptr<UIViewController>> _presentedViewControllers;

    friend class UIViewController;
    void addPresentedViewController(const std::shared_ptr<UIViewController>& controller);
    void removePresentedViewController(const std::shared_ptr<UIViewController>& controller);

    void sendTouchEvent(std::shared_ptr<UIEvent> event);
    void sendPressEvent(const std::shared_ptr<UIPressesEvent>& event);

    std::shared_ptr<UIFocusSystem> _focusSystem;
};

}
