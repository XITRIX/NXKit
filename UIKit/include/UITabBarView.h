#pragma once

#include <UIView.h>
#include <UITabBarItem.h>
#include <memory>
#include <optional>
#include <vector>

namespace NXKit {
enum class UITouchPhase;
class UITabBarView;
using UITabBar = UITabBarView;
namespace Internal { struct TabBarViewState; class TabBarViewAccess; }
class UITabBarDelegate {
public:
    virtual ~UITabBarDelegate() = default;
    virtual void tabBarDidSelect(const std::shared_ptr<UITabBar>& tabBar,
                                const std::shared_ptr<UITabBarItem>& item) {}
};

/** Apple-style floating tab bar. UITabBar is an alias for the portable view name.
    Items and selection follow UITabBar; comparison controls are private. */
class UITabBarView : public UIView {
public:
    static std::shared_ptr<UIView> init() { return new_shared<UITabBarView>(); }
    explicit UITabBarView(NXRect frame = {});
    ~UITabBarView() override;
    [[nodiscard]] std::vector<std::shared_ptr<UITabBarItem>> items() const;
    void setItems(std::vector<std::shared_ptr<UITabBarItem>> items, bool animated = false);
    [[nodiscard]] std::shared_ptr<UITabBarItem> selectedItem() const;
    void setSelectedItem(const std::shared_ptr<UITabBarItem>& item);
    [[nodiscard]] std::shared_ptr<UITabBarDelegate> delegate() const;
    void setDelegate(const std::shared_ptr<UITabBarDelegate>& delegate);
    [[nodiscard]] std::optional<UIColor> unselectedItemTintColor() const;
    void setUnselectedItemTintColor(std::optional<UIColor> color);
    void tintColorDidChange() override;
    void traitCollectionDidChange(std::shared_ptr<UITraitCollection> previous) override;
    void layoutSubviews() override;
    NXSize sizeThatFits(NXSize size) override;
    bool point(NXPoint insidePoint, UIEvent* event) override;
    std::vector<std::shared_ptr<UIFocusEnvironment>> preferredFocusEnvironments() override;
private:
    friend class Internal::TabBarViewAccess;
    std::unique_ptr<Internal::TabBarViewState> _state;
    void installItems(std::vector<std::shared_ptr<UITabBarItem>> items, bool animated);
    void refreshItems();
    void commitSelection(const std::shared_ptr<UITabBarItem>& item, bool animated);
    bool activateIndex(size_t index);
    void resetContact();
    void arrange();
    void animateSelectionPosition(float position);
    void beginLift();
    void releaseLift(bool cancelled = false);
    void advanceLift(double seconds);
    void observeTouch(UITouchPhase phase, const std::shared_ptr<UITouch>& touch);
    float positionForContact(NXFloat x) const;
    float positionForFinger(NXPoint point) const;
    void setSelectionPosition(float position);
    void setLifted(bool lifted, bool animated = false);
    void setComparisonContent(std::shared_ptr<UIImage> normal, std::shared_ptr<UIImage> selected);
    void setComparisonGeometry(NXRect frame);
};
}
