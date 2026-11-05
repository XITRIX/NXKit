#pragma once
#include <UIBarButtonItem.h>
#include <functional>
#include <vector>
namespace NXKit {
namespace Internal { class NavigationItemAccess; }
/** UIKit's out-of-line title policy. Automatic inherits the preceding item's
    request (the first item requests a large title). Inline mode is not yet supported. */
enum class UINavigationItemLargeTitleDisplayMode { automatic, always, never };
/** Style-independent description of a navigation bar's content. */
class UINavigationItem {
public:
    explicit UINavigationItem(std::string title = {}):_title(std::move(title)) {}
    [[nodiscard]] const std::string& title() const { return _title; }
    void setTitle(std::string title);
    [[nodiscard]] std::shared_ptr<UIView> titleView() const { return _titleView; }
    void setTitleView(std::shared_ptr<UIView> view);
    [[nodiscard]] std::vector<std::shared_ptr<UIBarButtonItem>> leftBarButtonItems() const { return _left; }
    [[nodiscard]] std::vector<std::shared_ptr<UIBarButtonItem>> rightBarButtonItems() const { return _right; }
    void setLeftBarButtonItems(std::vector<std::shared_ptr<UIBarButtonItem>> items,bool animated=false);
    void setRightBarButtonItems(std::vector<std::shared_ptr<UIBarButtonItem>> items,bool animated=false);
    [[nodiscard]] std::shared_ptr<UIBarButtonItem> leftBarButtonItem() const { return _left.empty()?nullptr:_left.front(); }
    [[nodiscard]] std::shared_ptr<UIBarButtonItem> rightBarButtonItem() const { return _right.empty()?nullptr:_right.front(); }
    void setLeftBarButtonItem(std::shared_ptr<UIBarButtonItem> item,bool animated=false);
    void setRightBarButtonItem(std::shared_ptr<UIBarButtonItem> item,bool animated=false);
    [[nodiscard]] bool hidesBackButton() const { return _hidesBack; }
    void setHidesBackButton(bool hidden,bool animated=false);
    [[nodiscard]] bool leftItemsSupplementBackButton() const { return _supplementsBack; }
    void setLeftItemsSupplementBackButton(bool supplement);
    [[nodiscard]] UINavigationItemLargeTitleDisplayMode largeTitleDisplayMode() const { return _largeTitleDisplayMode; }
    void setLargeTitleDisplayMode(UINavigationItemLargeTitleDisplayMode mode);
private:
    UINavigationItemLargeTitleDisplayMode _largeTitleDisplayMode=UINavigationItemLargeTitleDisplayMode::automatic;
    friend class Internal::NavigationItemAccess;
    struct Observer {std::weak_ptr<void> owner;std::function<void(bool)> changed;};
    std::vector<Observer> _observers;
    std::string _title;
    std::shared_ptr<UIView> _titleView;
    std::vector<std::shared_ptr<UIBarButtonItem>> _left,_right;
    bool _hidesBack=false,_supplementsBack=false,_explicitTitle=false;
    void changed(bool animated=false);
};
}
