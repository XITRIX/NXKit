#pragma once

#include <UIBarItem.h>
#include <UIColor.h>
#include <optional>

namespace NXKit {
class UITabBarItem : public UIBarItem {
public:
    UITabBarItem() = default;
    UITabBarItem(std::string title, std::shared_ptr<UIImage> image, std::intptr_t tag);
    UITabBarItem(std::string title, std::shared_ptr<UIImage> image,
                 std::shared_ptr<UIImage> selectedImage);
    [[nodiscard]] std::shared_ptr<UIImage> selectedImage() const { return _selectedImage; }
    void setSelectedImage(std::shared_ptr<UIImage> image);
    [[nodiscard]] std::optional<std::string> badgeValue() const { return _badgeValue; }
    void setBadgeValue(std::optional<std::string> value);
    [[nodiscard]] std::optional<UIColor> badgeColor() const { return _badgeColor; }
    void setBadgeColor(std::optional<UIColor> color);
private:
    std::shared_ptr<UIImage> _selectedImage;
    std::optional<std::string> _badgeValue;
    std::optional<UIColor> _badgeColor;
};
}
