#include "SwitchShaderBake.hpp"

#ifdef __SWITCH__
#include <UIKit.h>
#include <CADisplayLink.h>
#include <UIApplication.h>
#include <SDL3/SDL.h>
#include "Screens/GlassDemoViewController/GlassDemoScene.hpp"
#include "Screens/GlassDemoViewController/GlassStressView.hpp"
#include "Screens/GlassDemoViewController/LensTabBarView.hpp"
#include "Screens/TabBarDemoController/TabBarDemoController.hpp"
#include "Screens/IBTestController/IBTestController.h"
#include <cstdio>

namespace NXKit {
namespace {
bool requested = false;
bool exitTestRequested = false;
std::unique_ptr<CADisplayLink> exitTestLink;

// Device compilation fixture. It runs the real materials and renderer rather
// than substituting standalone fragment shaders for Graphite paint pipelines.
class ShaderBakeController final : public UIViewController {
public:
    void viewDidLoad() override {
        UIViewController::viewDidLoad();
        artwork = UIImage::fromRes("img/moonlight_store_logo.png");
        view()->setBackgroundColor(UIColor::secondarySystemBackground);
        showStage();
    }
    void viewDidAppear(bool animated) override {
        UIViewController::viewDidAppear(animated);
        pulse = std::make_unique<CADisplayLink>([this] { advance(); });
    }
    void viewWillDisappear(bool animated) override {
        pulse.reset();
        UIViewController::viewWillDisappear(animated);
    }
    void viewDidLayoutSubviews() override {
        UIViewController::viewDidLayoutSubviews();
        if (content) content->setFrame(view()->bounds());
        if (bar) bar->setFrame({120, 300, 720, 80});
    }
private:
    static constexpr int framesPerStage = 16;
    static constexpr int stageCount = GlassDemoCount + 8 + 3;
    int stage = 0, frame = 0;
    std::unique_ptr<CADisplayLink> pulse;
    std::shared_ptr<UIImage> artwork;
    std::shared_ptr<UIView> content;
    std::shared_ptr<GlassDemoScene> glass;
    std::shared_ptr<LensTabBarView> bar;
    std::shared_ptr<UIViewController> child;

    void showStage() {
        if (child) {
            child->willMoveToParent(nullptr);
            child->view()->removeFromSuperview();
            child->removeFromParent();
            child.reset();
        }
        if (content) content->removeFromSuperview();
        content.reset(); glass.reset(); bar.reset();
        if (stage < GlassDemoCount) {
            glass = new_shared<GlassDemoScene>(static_cast<GlassDemoID>(stage), artwork);
            content = glass;
        } else if (stage < GlassDemoCount + 8) {
            const int variant = stage - GlassDemoCount;
            content = new_shared<UIView>();
            content->setBackgroundColor(variant & 1 ? UIColor::white : UIColor::black);
            bar = new_shared<LensTabBarView>((variant & 2) != 0, (variant & 4) ? 4 : 3);
            bar->setFrame({120, 300, 720, 80});
            bar->lens()->setContentVibrancyEnabled((variant & 1) != 0);
            content->addSubview(bar);
        } else if (stage == GlassDemoCount + 8) {
            content = new_shared<GlassStressView>();
        } else {
            child = stage == GlassDemoCount + 9
                ? std::static_pointer_cast<UIViewController>(new_shared<TabBarDemoController>())
                : std::static_pointer_cast<UIViewController>(new_shared<IBTestController>());
            addChild(child);
            content = child->view();
            child->didMoveToParent(shared_from_base<UIViewController>());
        }
        content->setFrame(view()->bounds());
        view()->addSubview(content);
        std::printf("NXKIT_SHADER_BAKE_STAGE %d/%d\n", stage + 1, stageCount);
        std::fflush(stdout);
    }
    void advance() {
        CALayer::requestFramerate(60);
        if (glass) {
            glass->setProgress(static_cast<float>(frame) / (framesPerStage - 1));
            glass->setBackdrop(frame < framesPerStage / 2 ? 0 : 3);
            for (const auto& control : glass->buttonControls())
                control->setHighlighted((frame & 4) != 0);
        }
        if (bar) {
            bar->setLifted(frame >= framesPerStage / 2, false);
            bar->setSelectionPosition(static_cast<float>(frame) / (framesPerStage - 1) * 2);
        }
        if (++frame < framesPerStage) return;
        frame = 0;
        if (++stage < stageCount) { showStage(); return; }
        // The previous stage has already presented. Exit through normal
        // teardown, which drains work and saves the native cache and keys.
        std::puts("NXKIT_SHADER_BAKE_COMPLETE");
        std::fflush(stdout);
        UIApplication::shared->handleSDLQuit();
    }
};
} // namespace

bool switchShaderBakeRequested() { return requested; }
void requestSwitchShaderBake() { requested = true; }
void requestSwitchExitTest() { exitTestRequested = true; }
void installSwitchExitTest(std::function<void(size_t)> selectScreen) {
    if (!exitTestRequested) return;
    auto frames = std::make_shared<int>(0);
    exitTestLink = std::make_unique<CADisplayLink>([frames, selectScreen] {
        CALayer::requestFramerate(60);
        ++*frames;
        constexpr int stageFrames = 180;
        constexpr int testFrames = 8 * stageFrames;
        const int frame = (*frames - 1) % stageFrames;
        const int stage = (*frames - 1) / stageFrames;
        if (frame == 0 && *frames < testFrames) {
            const auto screen = static_cast<size_t>(stage);
            std::printf("NXKIT_NORMAL_EXIT_SCREEN %zu\n", screen);
            std::fflush(stdout);
            selectScreen(screen);
        }
        if (stage < 8 && frame >= 20 && frame <= 140) {
            SDL_Event event{};
            event.type = frame == 20 ? SDL_EVENT_FINGER_DOWN
                : frame == 140 ? SDL_EVENT_FINGER_UP : SDL_EVENT_FINGER_MOTION;
            event.tfinger.timestamp = SDL_GetTicksNS();
            event.tfinger.touchID = 123;
            event.tfinger.fingerID = 456;
            event.tfinger.x = .55f;
            event.tfinger.y = .7f - static_cast<float>((frame - 20) / 3) * .008f;
            SDL_PushEvent(&event);
        }
        if (*frames < testFrames) return;
        if (*frames == testFrames || *frames == testFrames + 20) {
            std::puts("NXKIT_NORMAL_EXIT_BUTTON_B");
            std::fflush(stdout);
            SDL_Event event{};
            event.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
            event.gbutton.which = 12345;
            event.gbutton.button = SDL_GAMEPAD_BUTTON_SOUTH;
            event.gbutton.down = true;
            SDL_PushEvent(&event);
            event.type = SDL_EVENT_GAMEPAD_BUTTON_UP;
            event.gbutton.down = false;
            SDL_PushEvent(&event);
        }
        if (!UIApplication::shared->isQuitRequested() && *frames < testFrames + 40) return;
        exitTestLink.reset();
        std::puts("NXKIT_NORMAL_EXIT_FAILED: B did not request quit");
        std::fflush(stdout);
        UIApplication::shared->handleSDLQuit();
    });
}
std::shared_ptr<UIViewController> makeSwitchShaderBakeController() {
    return new_shared<ShaderBakeController>();
}
} // namespace NXKit
#endif
