#pragma once
#include <UIKit.h>
#include <functional>

// Sample artwork and retained comparison controls around production UITabBarView.
class LensTabBarView final : public NXKit::UITabBarView {
public:
    explicit LensTabBarView(bool compact = false, size_t itemCount = 3);
    void setAccentColor(NXKit::UIColor color);
    void setSelectedIndex(size_t index, bool animated = false);
    size_t selectedIndex() const;
    void setSelectionPosition(float position);
    float selectionPosition() const;
    void setLifted(bool lifted, bool animated = false);
    bool isLifted() const;
    std::shared_ptr<NXKit::LensView> lens() const;
    std::shared_ptr<NXKit::BackdropEffectView> glass() const;
    const std::vector<std::shared_ptr<NXKit::UIControl>>& controls() const;
    void setComparisonContent(std::shared_ptr<NXKit::UIImage> normal, std::shared_ptr<NXKit::UIImage> selected);
    void setComparisonGeometry(NXKit::NXRect frame);
    std::function<void(size_t)> onSelectionChanged;
private:
    friend class LensTabBarTestHarness;
    void advanceLift(double seconds);
    bool hasPulse() const;
    void arrangeFixtures();
};
