#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace NXKit {
class UIViewController;

/** Portable tab selection capability. Null selection is expressed with optional
    instead of Apple's NSNotFound. Sectioned tabs and chrome belong to each kit. */
class UITabBarControllerInterface {
public:
    virtual ~UITabBarControllerInterface() = default;
    [[nodiscard]] virtual std::vector<std::shared_ptr<UIViewController>> viewControllers() const = 0;
    virtual void setViewControllers(std::vector<std::shared_ptr<UIViewController>> controllers) = 0;
    [[nodiscard]] virtual std::optional<std::size_t> selectedIndex() const = 0;
    virtual bool setSelectedIndex(std::size_t index) = 0;
    [[nodiscard]] virtual std::shared_ptr<UIViewController> selectedViewController() const = 0;
    virtual bool setSelectedViewController(const std::shared_ptr<UIViewController>& controller) = 0;
};
}
