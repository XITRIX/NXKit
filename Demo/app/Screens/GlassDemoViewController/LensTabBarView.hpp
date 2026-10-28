#pragma once
#include <UIKit.h>
#include <UITouch.h>
#include <functional>

// A comparison/demo composition, not a public UIKit tab controller. The platter
// is the existing regular glass; only the selection is a LensView.
class LensTabBarView final : public NXKit::UIView {
public:
    explicit LensTabBarView(bool compact = false, size_t itemCount = 3);
    void setAccentColor(NXKit::UIColor color);
    void setSelectedIndex(size_t index, bool animated = false);
    size_t selectedIndex() const { return _selected; }
    void setSelectionPosition(float position);
    float selectionPosition() const { return _position; }
    void setLifted(bool lifted, bool animated = false);
    bool isLifted() const { return _lifted; }
    std::shared_ptr<NXKit::LensView> lens() const { return _lens; }
    std::shared_ptr<NXKit::BackdropEffectView> glass() const { return _glass; }
    const std::vector<std::shared_ptr<NXKit::UIControl>>& controls() const { return _controls; }
    // Native-rendered content is a comparison-only input so typography does not
    // hide material error. The live Demo uses its own portable views instead.
    void setComparisonContent(std::shared_ptr<NXKit::UIImage> normal,
                              std::shared_ptr<NXKit::UIImage> selected);
    void setComparisonGeometry(NXKit::NXRect localLensFrame);
    std::function<void(size_t)> onSelectionChanged;
    void layoutSubviews() override;
private:
    void arrange();
    void observeTouch(NXKit::UITouchPhase phase, const std::shared_ptr<NXKit::UITouch>& touch);
    size_t _count;
    bool _compact, _lifted = false, _dragging = false;
    size_t _selected = 0, _dragStart = 0;
    float _position = 0, _selectionStep = 1;
    std::weak_ptr<NXKit::UITouch> _trackingTouch;
    NXKit::NXPoint _lastContact;
    NXKit::Timer _lastContactTime;
    std::optional<NXKit::NXRect> _comparisonFrame;
    std::shared_ptr<NXKit::BackdropEffectView> _glass;
    std::shared_ptr<NXKit::UIView> _normal, _selectedHost, _selectedContent;
    std::shared_ptr<NXKit::LensView> _lens;
    std::shared_ptr<NXKit::UIImageView> _normalImage, _selectedImage;
    std::vector<std::shared_ptr<NXKit::UIControl>> _controls;
    std::vector<std::shared_ptr<NXKit::UIView>> _normalSymbols, _selectedSymbols;
    std::vector<std::shared_ptr<NXKit::UILabel>> _normalLabels, _selectedLabels;
};
