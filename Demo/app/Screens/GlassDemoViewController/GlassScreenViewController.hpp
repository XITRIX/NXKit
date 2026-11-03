#pragma once
#include "GlassScreenView.hpp"
#include <UITabBarController.h>
#include <UINavigationController.h>
// A sequence of real navigation destinations for exercising glass group morphs.
class GlassNavigationDemoViewController final : public NXKit::UIViewController {
public:
    static constexpr size_t arrangementCount=8;
    explicit GlassNavigationDemoViewController(size_t arrangement=0);
    size_t arrangement() const { return _arrangement; }
    std::shared_ptr<NXKit::UIControl> action(const std::string& identifier);
    void loadView() override;
private:
    void push(size_t arrangement);
    void recordAction(const std::string& name);
    size_t _arrangement;
    size_t _presses=0;
};
class GlassListViewController final : public NXKit::UIViewController {
public:
    explicit GlassListViewController(size_t tab);
    std::function<void()> onClose;
    void loadView() override;
    void viewWillAppear(bool animated) override;
    std::shared_ptr<GlassScreenView> list();
private:
    size_t _tab;
    void updateAccent();
};
class GlassScreenViewController final : public NXKit::UITabBarController {
public:
    GlassScreenViewController();
    void willMoveToParent(const std::shared_ptr<NXKit::UIViewController>& parent) override;
    std::shared_ptr<GlassScreenView> selectedList();
    std::shared_ptr<NXKit::UINavigationController> selectedNavigationController();
private:
    void close();
};
