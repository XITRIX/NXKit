#pragma once
#include <UIBarItem.h>
#include <UIAction.h>
#include <UIColor.h>
#include <optional>

namespace NXKit {
class UIView;
enum class UIBarButtonItemStyle { plain=0, prominent=2, done=prominent };
enum class UIBarButtonSystemItem { done, cancel, edit, save, add, flexibleSpace, fixedSpace,
    compose, reply, action, organize, bookmarks, search, refresh, stop, camera, trash,
    play, pause, rewind, fastForward, undo, redo };
/** Portable bar-button metadata. UIAction replaces Objective-C target/selector. */
class UIBarButtonItem : public UIBarItem {
public:
    UIBarButtonItem() = default;
    UIBarButtonItem(std::string title, UIBarButtonItemStyle style, std::optional<UIAction> action = {});
    UIBarButtonItem(std::shared_ptr<UIImage> image, UIBarButtonItemStyle style, std::optional<UIAction> action = {});
    UIBarButtonItem(UIBarButtonSystemItem systemItem, std::optional<UIAction> action = {});
    explicit UIBarButtonItem(UIAction primaryAction);
    explicit UIBarButtonItem(std::shared_ptr<UIView> customView);
    [[nodiscard]] UIBarButtonItemStyle style() const { return _style; }
    void setStyle(UIBarButtonItemStyle style);
    [[nodiscard]] NXFloat width() const { return _width; }
    void setWidth(NXFloat width);
    [[nodiscard]] std::optional<UIAction> primaryAction() const { return _action; }
    void setPrimaryAction(std::optional<UIAction> action);
    [[nodiscard]] std::shared_ptr<UIView> customView() const { return _customView; }
    void setCustomView(std::shared_ptr<UIView> view);
    [[nodiscard]] std::optional<UIColor> tintColor() const { return _tintColor; }
    void setTintColor(std::optional<UIColor> color);
    [[nodiscard]] bool hidesSharedBackground() const { return _hidesSharedBackground; }
    void setHidesSharedBackground(bool hidden);
    [[nodiscard]] bool sharesBackground() const { return _sharesBackground; }
    void setSharesBackground(bool shares);
    [[nodiscard]] const std::optional<std::string>& identifier() const { return _identifier; }
    void setIdentifier(std::optional<std::string> identifier);
    static std::shared_ptr<UIBarButtonItem> fixedSpaceItemOfWidth(NXFloat width);
private:
    friend class Internal::BarItemAccess;
    std::optional<UIBarButtonSystemItem> _systemItem;
    UIBarButtonItemStyle _style=UIBarButtonItemStyle::plain;
    NXFloat _width=0;
    std::optional<UIAction> _action;
    std::shared_ptr<UIView> _customView;
    std::optional<UIColor> _tintColor;
    bool _hidesSharedBackground=false;
    bool _sharesBackground=true;
    std::optional<std::string> _identifier;
};
}
