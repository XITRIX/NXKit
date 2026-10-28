#pragma once
#include "LensTabBarView.hpp"
#include <map>
#include <set>

// Full-screen application composition used unchanged by the Demo and renderer.
// This is deliberately not a new UIKit navigation/list API.
class GlassScreenView final : public NXKit::UIView {
public:
    GlassScreenView();
    size_t selectedTab() const { return _tab->selectedIndex(); }
    void selectTab(size_t index);
    int addedItems() const { return _added; }
    int accent() const { return _accent; }
    int backdrop() const { return _backdrop; }
    bool showsDetails() const { return _details; }
    bool reversed() const { return _reversed; }
    size_t completedItems() const { return _completed.size(); }
    void addItem();
    void cycleAccent();
    void cycleBackdrop();
    void toggleDetails();
    void toggleOrder();
    void reset();
    void setScrollPosition(NXKit::NXFloat position);
    std::shared_ptr<NXKit::UIControl> action(const std::string& id) const;
    std::shared_ptr<LensTabBarView> tabBar() const { return _tab; }
    std::shared_ptr<NXKit::UIScrollView> scrollView() const { return _scroll; }
    NXKit::NXRect navigationFrame() const { return _navigation->frame(); }
    std::function<void()> onBack;
    void layoutSubviews() override;
    void safeAreaInsetsDidChange() override;
private:
    void rebuild();
    void arrange();
    NXKit::UIColor accentColor() const;
    int _added = 0, _accent = 0, _backdrop = 0;
    bool _details = false, _reversed = false;
    NXKit::NXFloat _topInset = -1;
    std::set<int> _completed;
    std::shared_ptr<NXKit::UIScrollView> _scroll;
    std::shared_ptr<NXKit::UIView> _content, _navigation;
    std::shared_ptr<NXKit::UIBlurView> _edgeBlur;
    std::shared_ptr<NXKit::UILabel> _title;
    std::shared_ptr<NXKit::BackdropEffectView> _plusGlass;
    std::shared_ptr<LensTabBarView> _tab;
    std::map<std::string,std::shared_ptr<NXKit::UIControl>> _actions;
};
