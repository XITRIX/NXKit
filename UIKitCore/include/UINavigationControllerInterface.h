#pragma once

#include <memory>
#include <vector>

namespace NXKit {
class UIViewController;

/** Portable navigation capability. Method names follow UINavigationController;
    bars, appearance and implementation-specific delegates belong to each kit. */
class UINavigationControllerInterface {
public:
    virtual ~UINavigationControllerInterface() = default;
    [[nodiscard]] virtual std::vector<std::shared_ptr<UIViewController>> viewControllers() const = 0;
    virtual void setViewControllers(std::vector<std::shared_ptr<UIViewController>> controllers, bool animated) = 0;
    virtual void pushViewController(const std::shared_ptr<UIViewController>& controller, bool animated) = 0;
    virtual std::shared_ptr<UIViewController> popViewController(bool animated) = 0;
    virtual std::vector<std::shared_ptr<UIViewController>> popToViewController(
        const std::shared_ptr<UIViewController>& controller, bool animated) = 0;
    virtual std::vector<std::shared_ptr<UIViewController>> popToRootViewController(bool animated) = 0;
    [[nodiscard]] virtual std::shared_ptr<UIViewController> topViewController() const = 0;
    [[nodiscard]] virtual std::shared_ptr<UIViewController> visibleViewController() const = 0;
};
}
