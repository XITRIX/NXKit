#include <UIBarItem.h>
#include <algorithm>
#include <utility>

using namespace NXKit;
void UIBarItem::notifyChanged() {
    std::erase_if(_observers, [](const auto& observer) { return observer.owner.expired(); });
    const auto observers = _observers;
    for (const auto& observer : observers) {
        if (const auto owner = observer.owner.lock()) observer.changed();
    }
}
void UIBarItem::setTitle(std::string value) {
    _hasExplicitTitle = true;
    if (_title == value) return;
    _title = std::move(value); notifyChanged();
}
void UIBarItem::setImage(std::shared_ptr<UIImage> value) {
    if (_image == value) return;
    _image = std::move(value); notifyChanged();
}
void UIBarItem::setEnabled(bool value) {
    if (_enabled == value) return;
    _enabled = value; notifyChanged();
}
void UIBarItem::setTag(std::intptr_t value) {
    if (_tag == value) return;
    _tag = value; notifyChanged();
}
