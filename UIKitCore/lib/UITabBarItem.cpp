#include <UITabBarItem.h>
#include <utility>

using namespace NXKit;
UITabBarItem::UITabBarItem(std::string title, std::shared_ptr<UIImage> image, std::intptr_t tag) {
    setTitle(std::move(title)); setImage(std::move(image)); setTag(tag);
}
UITabBarItem::UITabBarItem(std::string title, std::shared_ptr<UIImage> image,
                         std::shared_ptr<UIImage> selectedImage)
    : _selectedImage(std::move(selectedImage)) {
    setTitle(std::move(title)); setImage(std::move(image));
}
void UITabBarItem::setSelectedImage(std::shared_ptr<UIImage> value) {
    if (_selectedImage == value) return;
    _selectedImage = std::move(value); notifyChanged();
}
void UITabBarItem::setBadgeValue(std::optional<std::string> value) {
    if (_badgeValue == value) return;
    _badgeValue = std::move(value); notifyChanged();
}
void UITabBarItem::setBadgeColor(std::optional<UIColor> value) {
    if (_badgeColor == value) return;
    _badgeColor = std::move(value); notifyChanged();
}
