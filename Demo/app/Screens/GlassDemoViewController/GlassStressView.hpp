#pragma once

#include <BackdropEffectView.h>
#include <CADisplayLink.h>
#include <UIControl.h>
#include <UILabel.h>
#include <UIScrollView.h>
#include <UIViewController.h>
#include <vector>

namespace NXKit {

// Shared by the interactive Demo and the real-run-loop performance replay.
class GlassStressView final : public UIView {
public:
    static constexpr int railCount = 8, cardsPerRail = 12;
    GlassStressView();
    void layoutSubviews() override;
    void changeBackdrop();
    void resetOffsets();
    std::shared_ptr<UIScrollView> verticalScroll() const { return _vertical; }
    const auto& horizontalScrolls() const { return _rails; }
    int activations() const { return _activations; }
    int backdropChanges() const { return _backdrop; }
private:
    std::shared_ptr<UIScrollView> _vertical;
    std::shared_ptr<UIView> _content;
    std::shared_ptr<BackdropEffectView> _toolbar;
    std::shared_ptr<UIControl> _backdropButton;
    std::shared_ptr<UILabel> _status;
    std::vector<std::shared_ptr<UILabel>> _headings;
    std::vector<std::shared_ptr<UIScrollView>> _rails;
    int _activations = 0, _backdrop = 0;
};

class GlassStressViewController final : public UIViewController {
public:
    GlassStressViewController() { setTitle("Glass stress test"); }
    void loadView() override { setView(new_shared<GlassStressView>()); }
    void viewDidAppear(bool animated) override {
        UIViewController::viewDidAppear(animated);
        _displayLink = std::make_unique<CADisplayLink>([] { CALayer::requestFramerate(60); });
    }
    void viewWillDisappear(bool animated) override {
        _displayLink.reset();
        UIViewController::viewWillDisappear(animated);
    }
private:
    std::unique_ptr<CADisplayLink> _displayLink;
};

}
