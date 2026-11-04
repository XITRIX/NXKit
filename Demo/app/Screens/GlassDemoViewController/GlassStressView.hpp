#pragma once

#include <BackdropEffectView.h>
#include <CADisplayLink.h>
#include <UIControl.h>
#include <UILabel.h>
#include <UIScrollView.h>
#include <UIViewController.h>
#include <SkiaCtx.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <string_view>

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
        const auto replay = std::getenv("NXKIT_GLASS_STRESS_REPLAY");
        const bool automatic = replay && std::string_view(replay) == "1";
        _displayLink = std::make_unique<CADisplayLink>([
            weak = std::weak_ptr<GlassStressView>(std::static_pointer_cast<GlassStressView>(view())),
            start = std::chrono::steady_clock::now(), automatic
        ] {
            CALayer::requestFramerate(SkiaCtx::main()->screenFrameRate());
            const auto screen = weak.lock();
            if (!automatic || !screen) return;
            // Two stationary seconds, then a ten-second down/up sweep. Use
            // elapsed time so profiling at different frame rates has equal speed.
            const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
            const auto phase = std::fmod(elapsed,12.0);
            const auto travel = phase < 2 ? 0.0 : 1-std::abs(2*(phase-2)/10-1);
            const auto scroll = screen->verticalScroll();
            scroll->setContentOffset({0,NXFloat(travel*std::max(0.f,scroll->contentSize().height-scroll->bounds().height()))},false);
            for (const auto& rail : screen->horizontalScrolls())
                rail->setContentOffset({NXFloat(travel*std::max(0.f,rail->contentSize().width-rail->bounds().width())),0},false);
        });
    }
    void viewWillDisappear(bool animated) override {
        _displayLink.reset();
        UIViewController::viewWillDisappear(animated);
    }
private:
    std::unique_ptr<CADisplayLink> _displayLink;
};

}
