#include "include/core/SkSurface.h"
#include <DispatchQueue.h>
#include <NXNavigationController.h>
#include <NXTabBarController.h>
#include <SDL3/SDL.h>
#include <SkiaCtx.h>
#include <UIApplication.h>
#ifdef __APPLE__
#include "include/ports/SkFontMgr_mac_ct.h"
#endif
#include <functional>
#include <iostream>

namespace NXKit {
bool applicationRunLoop();
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,
                                                                     std::map<std::string, std::any>) {
    return true;
}
class DispatchQueueTestHarness {
public:
    static void pump() { DispatchQueue::main()->performAll(); }
};
namespace {
int failures = 0;
void expect(bool value, const char* message) {
    if (!value) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}
class Context final : public SkiaCtx {
public:
    NXSize size{390, 844};
    Context() {
#ifdef __APPLE__
        fontMgr = SkFontMgr_New_CoreText(nullptr);
#endif
    }
    NXSize getSize() override { return size; }
    sk_sp<SkSurface> getBackbufferSurface() override { return nullptr; }
    void swapBuffers() override {}
    void sleepForNextFrame() override {}
    bool platformRunLoop(std::function<bool()>) override { return false; }

protected:
    skgpu::graphite::Context* graphiteContext() override { return nullptr; }
    skgpu::graphite::Recorder* graphiteRecorder() override { return nullptr; }
};
class Screen final : public UIViewController {
public:
    int loads = 0, appears = 0, disappears = 0, state = 0;
    void loadView() override {
        ++loads;
        setView(new_shared<UIView>());
    }
    void viewDidAppear(bool) override { ++appears; }
    void viewDidDisappear(bool) override { ++disappears; }
};
class Delegate final : public NXTabBarControllerDelegate {
public:
    bool allow = true;
    int didSelect = 0;
    bool tabBarControllerShouldSelect(const std::shared_ptr<NXTabBarController>&,
                                      const std::shared_ptr<UIViewController>&) override {
        return allow;
    }
    void tabBarControllerDidSelect(const std::shared_ptr<NXTabBarController>&,
                                   const std::shared_ptr<UIViewController>&) override {
        ++didSelect;
    }
};
void settle() {
    for (int pass = 0; pass < 8; ++pass) {
        const auto layers = UIView::layersWithAnimations;
        for (const auto& layer : layers)
            layer->animateAt(Timer(1'000'000));
        DispatchQueueTestHarness::pump();
    }
}
void frame() {
    expect(applicationRunLoop(), "the application frame completes");
}
void back() {
    SDL_Event down{};
    down.type = SDL_EVENT_KEY_DOWN;
    down.key.key = SDLK_ESCAPE;
    down.key.scancode = SDL_SCANCODE_ESCAPE;
    down.key.down = true;
    SDL_Event up = down;
    up.type = SDL_EVENT_KEY_UP;
    up.key.down = false;
    expect(SDL_PushEvent(&down) && SDL_PushEvent(&up), "Back input is queued");
    UIApplication::shared->handleEventsIfNeeded();
}
std::shared_ptr<NXTabBarButton> button(const std::shared_ptr<NXTabBar>& bar, const std::string& title) {
    for (const auto& item : bar->subviews().front()->subviews()) {
        const auto candidate = std::dynamic_pointer_cast<NXTabBarButton>(item);
        if (candidate && candidate->text() == title)
            return candidate;
    }
    throw std::logic_error("Missing tab button");
}
void navigationCases(const std::shared_ptr<Context>& ctx) {
    auto first = new_shared<Screen>(), second = new_shared<Screen>();
    first->setTitle("First");
    second->setTitle("Second");
    auto tabs = new_shared<NXTabBarController>(NXTabBarController::ViewControllerSection{first, second});
    auto nav = new_shared<NXNavigationController>(tabs);
    auto window = new_shared<UIWindow>();
    UIApplication::shared->delegate->window = window;
    window->setRootViewController(nav);
    window->makeKeyAndVisible();
    frame();
    expect(tabs->tabBar()->frame().width() == 390, "compact sidebar occupies the full screen width");
    expect(nav->topViewController() == tabs && first->loads == 0,
           "compact starts on the list without loading selected content");
    auto firstButton = button(tabs->tabBar(), "First"), secondButton = button(tabs->tabBar(), "Second");
    window->focusSystem()->requestFocusUpdate(secondButton);
    expect(tabs->selectedViewController() == first && nav->topViewController() == tabs,
           "moving controller focus in compact does not select or push");
    auto delegate = new_shared<Delegate>();
    tabs->delegate = delegate;
    delegate->allow = false;
    secondButton->primaryAction->perform();
    expect(tabs->selectedViewController() == first && nav->topViewController() == tabs,
           "a vetoed compact selection does not navigate");
    delegate->allow = true;
    auto item = nav->navigationItemFor(second);
    item->setTitleOverride("Second item");
    secondButton->primaryAction->perform();
    expect(nav->topViewController() == second && second->parent().lock() == nav,
           "compact selection uses show with the actual selected controller");
    expect(nav->topNavigationItem() == item, "compact navigation preserves the tab's navigation item");
    expect(nav->isTransitioning(), "the compact selection uses the normal navigation animation");
    second->state = 42;
    const auto originalView = second->view();
    ctx->size = {1000, 800};
    frame(); // Expand while the push is still in flight.
    settle();
    frame();
    expect(nav->topViewController() == tabs && tabs->selectedViewController() == second,
           "expanding during a push returns to the sidebar and selected tab");
    expect(second->parent().lock() == tabs && second->view() == originalView && second->state == 42 &&
               second->loads == 1,
           "expansion reparents the same controller and view without losing state");
    expect(second->view()->frame().width() == 1000 - tabs->tabBarWidth(),
           "regular content uses the width beside the sidebar");
    expect(second->appears == second->disappears + 1,
           "compact-to-regular transfer balances selected-screen appearances");
    expect(delegate->didSelect == 1, "size-class adaptation does not emit another user selection");
    tabs->setTabBarWidth(300);
    frame();
    expect(tabs->tabBar()->frame().width() == 300, "regular preserves configurable sidebar width");
    tabs->tabBar()->setSelectionFollowsFocus(false);
    ctx->size = {390, 844};
    frame();
    settle();
    expect(nav->topViewController() == tabs && tabs->tabBar()->frame().width() == 390,
           "collapsing shows the full-width list with no automatic push");
    expect(tabs->selectedViewController() == second && second->parent().lock() == tabs,
           "collapsing preserves the selected tab");
    button(tabs->tabBar(), "First")->primaryAction->perform();
    settle();
    expect(nav->topViewController() == first, "a different compact item opens its screen");
    back();
    settle();
    frame();
    expect(nav->topViewController() == tabs && first->parent().lock() == tabs &&
               tabs->selectedViewController() == first,
           "Back restores containment and keeps compact selection");
    button(tabs->tabBar(), "First")->primaryAction->perform();
    settle();
    expect(nav->topViewController() == first && nav->viewControllers().size() == 2,
           "reopening the same tab creates no duplicate navigation entry");
    tabs->setViewControllerSections({{second, first}});
    expect(tabs->selectedIndex() == 1 && first->parent().lock() == nav,
           "reordering tabs retains the open compact selection");
    auto nested = new_shared<Screen>();
    first->show(nested);
    settle();
    expect(nav->topViewController() == nested, "a compact screen can push a nested destination normally");
    ctx->size = {1000, 800};
    frame();
    settle();
    frame();
    expect(nav->topViewController() == nested &&
               nav->viewControllers() == std::vector<std::shared_ptr<UIViewController>>{tabs, nested},
           "expansion removes only the compact tab entry and retains nested pushes");
    expect(tabs->selectedViewController() == first && first->parent().lock() == tabs,
           "the selected tab is restored behind a nested destination");
    nav->popViewController(false);
    settle();
    frame();
    expect(nav->topViewController() == tabs && !button(tabs->tabBar(), "Second")->selectionFollowsFocus(),
           "returning to regular preserves the configured focus-selection policy");
    ctx->size = {390, 844};
    frame();
    button(tabs->tabBar(), "Second")->primaryAction->perform();
    tabs->setViewControllers({});
    settle();
    frame();
    expect(nav->topViewController() == tabs && !tabs->selectedViewController() && second->parent().expired(),
           "clearing tabs during a compact push closes and detaches the destination safely");
    tabs->setViewControllers({first, second});
    frame();
    button(tabs->tabBar(), "First")->primaryAction->perform();
    settle();
    tabs->setSelectedViewController(second);
    settle();
    expect(nav->topViewController() == second && first->parent().lock() == tabs,
           "programmatic selection updates an already open compact destination");
    nav->popViewController(false);
    settle();
    frame();
    expect(nav->viewControllers().size() == 1, "Back after a compact switch leaves only the list");
    button(tabs->tabBar(), "Second")->primaryAction->perform();
    ctx->size = {1000, 800};
    frame();
    ctx->size = {390, 844};
    frame();
    settle();
    frame();
    expect(nav->topViewController() == second && second->parent().lock() == nav,
           "the latest size class wins when expansion is interrupted by collapse during a push");
    back();
    ctx->size = {1000, 800};
    frame();
    settle();
    frame();
    expect(nav->topViewController() == tabs && second->parent().lock() == tabs &&
               tabs->selectedViewController() == second,
           "expanding during Back preserves the selected tab in regular content");
    window->setRootViewController(nullptr);
    settle();
}
void modalCases(const std::shared_ptr<Context>& ctx) {
    ctx->size = {390, 844};
    auto screen = new_shared<Screen>();
    screen->setTitle("Modal");
    auto tabs = new_shared<NXTabBarController>(NXTabBarController::ViewControllerSection{screen});
    auto window = new_shared<UIWindow>();
    UIApplication::shared->delegate->window = window;
    window->setRootViewController(tabs);
    window->makeKeyAndVisible();
    frame();
    button(tabs->tabBar(), "Modal")->primaryAction->perform();
    settle();
    expect(tabs->presentedViewController() == screen,
           "show falls back to presentation when no navigation controller exists");
    screen->state = 17;
    ctx->size = {1000, 800};
    frame();
    settle();
    frame();
    expect(!tabs->presentedViewController() && tabs->selectedViewController() == screen &&
               screen->parent().lock() == tabs && screen->state == 17,
           "expanding a standalone compact presentation restores its selected content");
    ctx->size = {390, 844};
    frame();
    button(tabs->tabBar(), "Modal")->primaryAction->perform();
    settle();
    screen->dismiss(false);
    settle();
    frame();
    expect(screen->parent().lock() == tabs && tabs->selectedViewController() == screen,
           "modal Back returns the selected controller to the tab container");
    button(tabs->tabBar(), "Modal")->primaryAction->perform();
    settle();
    expect(tabs->presentedViewController() == screen, "a dismissed standalone tab can be opened again");
    screen->dismiss(false);
    settle();
    screen->setModalPresentationStyle(UIModalPresentationStyle::overFullScreen);
    button(tabs->tabBar(), "Modal")->primaryAction->perform();
    settle();
    back();
    settle();
    expect(!tabs->presentedViewController() && screen->parent().lock() == tabs,
           "overlay Back restores containment without relying on the list's appearance callback");
    window->setRootViewController(nullptr);
}
} // namespace
int runAdaptiveTabTests() {
    auto ctx = std::make_shared<Context>();
    SkiaCtx::_main = ctx;
    UIApplication::shared = new_shared<UIApplication>();
    UIApplication::shared->delegate = std::make_shared<UIApplicationDelegate>();
    expect(SDL_InitSubSystem(SDL_INIT_EVENTS), "event subsystem initializes");
    navigationCases(ctx);
    modalCases(ctx);
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
    UIApplication::shared.reset();
    SkiaCtx::_main.reset();
    if (!failures)
        std::cout << "Adaptive tab controller tests passed\n";
    return failures ? 1 : 0;
}
} // namespace NXKit
int main() {
    return NXKit::runAdaptiveTabTests();
}
