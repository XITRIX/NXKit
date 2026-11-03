#pragma once
#include <UIKit.h>
#include <map>
#include <set>

// Existing Demo content list. Bars and navigation belong to its containers.
class GlassScreenView final : public NXKit::UIView {
public:
    GlassScreenView();
    size_t selectedTab() const { return _selectedTab; }
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
    std::shared_ptr<NXKit::UIScrollView> scrollView() const { return _scroll; }
    std::function<void(NXKit::UIColor)> onAccentChanged;
    std::function<void()> onNavigationDemo;
    NXKit::UIColor accentColor() const;
    void layoutSubviews() override;
    void safeAreaInsetsDidChange() override;
private:
    void rebuild();
    void arrange();
    size_t _selectedTab=0;
    int _added = 0, _accent = 0, _backdrop = 0;
    bool _details = false, _reversed = false;
    NXKit::NXFloat _topInset = -1;
    std::set<int> _completed;
    std::shared_ptr<NXKit::UIScrollView> _scroll;
    std::shared_ptr<NXKit::UIView> _content;
    std::map<std::string,std::shared_ptr<NXKit::UIControl>> _actions;
};
