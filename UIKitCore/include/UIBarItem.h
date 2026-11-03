#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NXKit {
class UIImage;
namespace Internal { class BarItemAccess; }

/** Shared, style-independent metadata for bar items. */
class UIBarItem {
public:
    virtual ~UIBarItem() = default;
    [[nodiscard]] const std::string& title() const { return _title; }
    void setTitle(std::string title);
    [[nodiscard]] std::shared_ptr<UIImage> image() const { return _image; }
    void setImage(std::shared_ptr<UIImage> image);
    [[nodiscard]] bool isEnabled() const { return _enabled; }
    void setEnabled(bool enabled);
    [[nodiscard]] std::intptr_t tag() const { return _tag; }
    void setTag(std::intptr_t tag);
protected:
    void notifyChanged();
private:
    friend class Internal::BarItemAccess;
    struct Observer { std::weak_ptr<void> owner; std::function<void()> changed; };
    std::vector<Observer> _observers;
    std::string _title;
    std::shared_ptr<UIImage> _image;
    std::intptr_t _tag = 0;
    bool _enabled = true;
    bool _hasExplicitTitle = false;
};
}
