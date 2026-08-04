#include <BackdropEffectView.h>
#include <NXTabBarController.h>
#include <NXNavigationController.h>
#include <NXResponderAction.h>
#include <SkiaCtx.h>
#include <UIApplication.h>
#include <UIApplicationDelegate.h>
#include <UIControl.h>
#include <UIGlassEffect.h>
#include <UIPress.h>
#include <UIWindow.h>

#include <SDL3/SDL.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using namespace NXKit;

namespace NXKit {

bool applicationRunLoop();

bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(
    UIApplication*,
    std::map<std::string, std::any>
) {
    return true;
}

} // namespace NXKit

namespace {

int failures = 0;

class TestSkiaContext final : public SkiaCtx {
public:
    UIUserInterfaceStyle themeMode = UIUserInterfaceStyle::light;

    sk_sp<SkSurface> getBackbufferSurface() override { return nullptr; }
    NXSize getSize() override { return { 1280, 720 }; }
    void swapBuffers() override {}
    UIUserInterfaceStyle getThemeMode() override { return themeMode; }
    bool platformRunLoop(std::function<bool()> loop) override { return loop(); }

protected:
    skgpu::graphite::Context* graphiteContext() override { return nullptr; }
    skgpu::graphite::Recorder* graphiteRecorder() override { return nullptr; }
};

void expect(bool condition, const std::string& message) {
    if (condition) {
        return;
    }
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}

std::vector<NXFloat> glassTint(const std::shared_ptr<BackdropEffectView>& view) {
    const auto tint = view->effect().uniform("tint");
    if (!tint || tint->size() != 4) {
        expect(false, "regular glass exposes a four-component tint uniform");
        return std::vector<NXFloat>(4);
    }
    return *tint;
}

NXFloat glassScalar(
    const std::shared_ptr<BackdropEffectView>& view,
    const std::string& name
) {
    const auto value = view->effect().uniform(name);
    if (!value || value->size() != 1) {
        expect(false, "regular glass exposes scalar uniform '" + name + "'");
        return 0;
    }
    return value->front();
}

void registerExitAction(
    const std::shared_ptr<UIWindow>& window,
    const std::shared_ptr<UIApplication>& application
) {
    NXResponderAction {
        .button = NXActionButton::b,
        .isEnabled = true,
        .action = UIAction("Exit", [
            weakApplication = std::weak_ptr<UIApplication>(application)
        ]() {
            if (const auto app = weakApplication.lock()) {
                app->handleSDLQuit();
            }
        }),
        .identifier = "NXKit.tests.application.exit",
        .priority = -100,
    }.registerOn(window);
}

class RecordingViewController final : public UIViewController {
public:
    int addedToParentCount = 0;
    int removedFromParentCount = 0;

    void didMoveToParent(std::shared_ptr<UIViewController> parent) override {
        UIViewController::didMoveToParent(parent);
        if (parent) {
            ++addedToParentCount;
        } else {
            ++removedFromParentCount;
        }
    }
};

class RecordingTraitView final : public UIView {
public:
    int traitChangeCount = 0;

    void traitCollectionDidChange(
        std::shared_ptr<UITraitCollection> previousTraitCollection
    ) override {
        UIView::traitCollectionDidChange(std::move(previousTraitCollection));
        ++traitChangeCount;
    }
};

class WideIntrinsicView final : public UIView {
public:
    NXSize sizeThatFits(NXSize size) override {
        return { 4000, size.height };
    }
};

class RecordingDelegate final : public NXTabBarControllerDelegate {
public:
    bool allowsSelection = true;
    int shouldSelectCount = 0;
    int didSelectCount = 0;

    bool tabBarControllerShouldSelect(
        const std::shared_ptr<NXTabBarController>&,
        const std::shared_ptr<UIViewController>&
    ) override {
        ++shouldSelectCount;
        return allowsSelection;
    }

    void tabBarControllerDidSelect(
        const std::shared_ptr<NXTabBarController>&,
        const std::shared_ptr<UIViewController>&
    ) override {
        ++didSelectCount;
    }
};

class BackConsumingControl final : public UIControl {
public:
    bool consumesBack = false;
    int consumedBackCount = 0;
    int focusGainCount = 0;
    int focusLossCount = 0;
    int focusMovementFailureCount = 0;

    void willGainFocus() override {
        ++focusGainCount;
        UIControl::willGainFocus();
    }

    void willLoseFocus() override {
        ++focusLossCount;
        UIControl::willLoseFocus();
    }

    void focusFailedToMove(UIFocusHeading heading) override {
        ++focusMovementFailureCount;
        UIControl::focusFailedToMove(heading);
    }

    void pressesEnded(
        std::set<std::shared_ptr<UIPress>> presses,
        std::shared_ptr<UIPressesEvent> event
    ) override {
        const bool isBackPress = std::any_of(
            presses.begin(),
            presses.end(),
            [](const std::shared_ptr<UIPress>& press) {
                return press && press->type() == UIPressType::menu;
            }
        );
        if (consumesBack && isBackPress) {
            ++consumedBackCount;
            return;
        }
        UIControl::pressesEnded(std::move(presses), std::move(event));
    }
};

class FocusableViewController final : public UIViewController {
public:
    std::shared_ptr<BackConsumingControl> control;

    void loadView() override {
        auto rootView = new_shared<UIView>();
        control = new_shared<BackConsumingControl>();
        rootView->addSubview(control);
        setView(rootView);
    }
};

class TraitConfiguredWindow final : public UIWindow {
public:
    void installTraitCollection(const std::shared_ptr<UITraitCollection>& traits) {
        const auto previousTraits = _traitCollection;
        _traitCollection = traits;
        traitCollectionDidChange(previousTraits);
    }
};

bool hierarchyHasTraitCollection(const std::shared_ptr<UIView>& view) {
    if (!view || !view->traitCollection()) {
        return false;
    }
    return std::all_of(
        view->subviews().begin(),
        view->subviews().end(),
        [](const std::shared_ptr<UIView>& subview) {
            return hierarchyHasTraitCollection(subview);
        }
    );
}

bool sendKeyPress(
    const std::shared_ptr<UIApplication>& application,
    SDL_Keycode keycode,
    SDL_Scancode scancode
) {
    SDL_Event keyDown {};
    keyDown.type = SDL_EVENT_KEY_DOWN;
    keyDown.key.key = keycode;
    keyDown.key.scancode = scancode;
    keyDown.key.down = true;

    SDL_Event keyUp = keyDown;
    keyUp.type = SDL_EVENT_KEY_UP;
    keyUp.key.down = false;

    if (!SDL_PushEvent(&keyDown) || !SDL_PushEvent(&keyUp)) {
        return false;
    }
    application->handleEventsIfNeeded();
    return true;
}

} // namespace

int main() {
    auto skiaContext = std::make_shared<TestSkiaContext>();
    SkiaCtx::_main = skiaContext;

    auto wideContentController = new_shared<UIViewController>();
    wideContentController->setTitle("Wide content");
    wideContentController->setView(new_shared<WideIntrinsicView>());
    auto boundedTabController = new_shared<NXTabBarController>(
        NXTabBarController::ViewControllerSection { wideContentController }
    );
    boundedTabController->view()->setFrame(NXRect(0, 0, 1280, 720));
    boundedTabController->view()->layoutIfNeeded();
    expect(
        wideContentController->view()->frame()
            == NXRect(
                0,
                0,
                1280 - boundedTabController->tabBarWidth(),
                720
            ),
        "the content column uses the remaining width instead of a child's intrinsic width"
    );

    auto first = new_shared<RecordingViewController>();
    auto second = new_shared<RecordingViewController>();
    auto third = new_shared<RecordingViewController>();
    first->setTitle("First");
    second->setTitle("Second");
    third->setTitle("Third");

    NXTabBarController::ViewControllerSections sections = {
        {},
        { first, second }
    };
    auto controller = new_shared<NXTabBarController>(sections);

    expect(controller->selectedIndex() == 0, "the first non-empty tab is selected initially");
    expect(
        controller->selectedIndexPath() == std::optional<IndexPath>(IndexPath(0, 1)),
        "the initial section and item are preserved"
    );
    expect(controller->selectedViewController() == first, "the selected controller is queryable");
    expect(controller->viewControllers().size() == 2, "the flat controller list skips empty sections");

    controller->loadViewIfNeeded();
    expect(first->parent().lock() == controller, "the selected controller is contained after loading");
    expect(second->parent().lock() == controller, "unselected controllers remain children of the tab container");
    expect(first->addedToParentCount == 1, "the selected controller receives didMoveToParent");
    expect(second->addedToParentCount == 1, "an unselected controller receives didMoveToParent");
    expect(
        controller->tabBar()->selectedIndexPath() == controller->selectedIndexPath(),
        "the visual tab selection matches the controller selection"
    );
    expect(
        controller->tabBar()->focusTrackingMode()
            == UIScrollViewFocusTrackingMode::centered,
        "the tab bar centers focused items within its scroll limits"
    );

    auto delegate = new_shared<RecordingDelegate>();
    controller->delegate = delegate;
    int publicSelectionCount = 0;
    controller->tabBar()->selectionDidChange = [&publicSelectionCount](const IndexPath&) {
        ++publicSelectionCount;
    };

    delegate->allowsSelection = false;
    expect(
        !controller->tabBar()->activateItemAt(IndexPath(1, 1)),
        "a delegate can reject user-driven selection"
    );
    expect(controller->selectedViewController() == first, "rejected selection preserves content");
    expect(publicSelectionCount == 0, "rejected selection emits no public change callback");

    delegate->allowsSelection = true;
    expect(
        controller->tabBar()->activateItemAt(IndexPath(1, 1)),
        "an allowed user-driven selection succeeds"
    );
    expect(controller->selectedViewController() == second, "allowed selection changes content");
    expect(first->parent().lock() == controller, "the previous tab remains a child of the container");
    expect(second->parent().lock() == controller, "the new selected controller is contained");
    expect(first->removedFromParentCount == 0, "selection does not change controller containment");
    expect(delegate->shouldSelectCount == 2, "the delegate is consulted for both user selections");
    expect(delegate->didSelectCount == 1, "the delegate is notified only for accepted selection");
    expect(publicSelectionCount == 1, "accepted selection emits the public change callback");

    expect(
        controller->tabBar()->activateItemAt(IndexPath(1, 1)),
        "reselecting the active tab is still a user selection"
    );
    expect(delegate->shouldSelectCount == 3, "reselection consults the delegate");
    expect(delegate->didSelectCount == 2, "reselection notifies the delegate");
    expect(publicSelectionCount == 2, "reselection emits the public callback");

    expect(!controller->setSelectedIndex(9), "an invalid flat index is rejected");
    expect(
        !controller->setSelectedIndexPath(IndexPath(-1, 0)),
        "a negative index path is rejected"
    );
    expect(controller->selectedViewController() == second, "invalid selection does not mutate state");

    controller->setViewControllerSections({ { second, third } });
    expect(controller->selectedViewController() == second, "reconfiguration preserves a surviving selection");
    expect(controller->selectedIndex() == 0, "the preserved selection receives its new flat index");
    expect(first->parent().expired(), "removed tabs are detached from the container");
    expect(first->removedFromParentCount == 1, "removed tabs receive didMoveToParent(nullptr)");
    expect(third->parent().lock() == controller, "new tabs are attached to the container");

    bool duplicateWasRejected = false;
    try {
        controller->setViewControllerSections({ { first, first } });
    } catch (const std::invalid_argument&) {
        duplicateWasRejected = true;
    }
    expect(duplicateWasRejected, "duplicate controllers are rejected visibly");

    bool nullWasRejected = false;
    try {
        controller->setViewControllerSections({ { nullptr } });
    } catch (const std::invalid_argument&) {
        nullWasRejected = true;
    }
    expect(nullWasRejected, "null controllers are rejected visibly");

    bool invalidWidthWasRejected = false;
    try {
        controller->setTabBarWidth(0);
    } catch (const std::invalid_argument&) {
        invalidWidthWasRejected = true;
    }
    expect(invalidWidthWasRejected, "non-positive tab bar widths are rejected visibly");

    controller->setViewControllerSections({});
    expect(!controller->selectedIndex().has_value(), "empty content has no selected index");
    expect(!controller->selectedIndexPath().has_value(), "empty content has no selected index path");
    expect(!controller->selectedViewController(), "empty content has no selected controller");
    expect(second->parent().expired(), "clearing tabs detaches the selected controller");
    expect(third->parent().expired(), "clearing tabs detaches unselected controllers");
    expect(!controller->tabBar()->selectedIndexPath(), "clearing tabs clears the visual selection");

    auto focusableController = new_shared<FocusableViewController>();
    focusableController->setTitle("Focusable");
    auto nestedNavigationController = new_shared<NXNavigationController>(
        focusableController
    );
    nestedNavigationController->setTitle("Focusable");
    auto otherFocusableController = new_shared<FocusableViewController>();
    otherFocusableController->setTitle("Other");
    auto focusTabController = new_shared<NXTabBarController>(
        NXTabBarController::ViewControllerSection {
            nestedNavigationController,
            otherFocusableController,
        }
    );
    auto focusNavigationController = new_shared<NXNavigationController>(
        focusTabController
    );
    auto focusWindow = new_shared<TraitConfiguredWindow>();
    focusWindow->installTraitCollection(UITraitCollection::current());
    focusWindow->setFrame(NXRect(0, 0, 1280, 720));
    focusWindow->setRootViewController(focusNavigationController);
    focusNavigationController->view()->setFrame(focusWindow->bounds());
    focusWindow->addSubview(focusNavigationController->view());
    focusWindow->updateFocus();
    focusNavigationController->defaultActionsWidget()->refresh();

    auto focusedView = std::dynamic_pointer_cast<UIView>(
        focusWindow->focusSystem()->focusedItem().lock()
    );
    expect(
        focusedView && focusedView->isDescendantOf(focusTabController->tabBar()),
        "tab-bar focus starts on a tab item"
    );

    auto application = std::make_shared<UIApplication>();
    auto applicationDelegate = std::make_shared<UIApplicationDelegate>();
    applicationDelegate->window = focusWindow;
    application->delegate = applicationDelegate;
    application->keyWindow = focusWindow;
    UIApplication::shared = application;
    registerExitAction(focusWindow, application);

    expect(applicationRunLoop(), "the initial light trait update completes");
    auto presentationChrome = new_shared<RecordingTraitView>();
    focusWindow->addSubview(presentationChrome);
    presentationChrome->traitChangeCount = 0;

    auto presentedRoot = new_shared<UIView>();
    auto presentedGlass = new_shared<BackdropEffectView>(
        UIGlassEffect(UIGlassEffect::Style::regular)
    );
    presentedGlass->setFrame(NXRect(0, 0, 520, 320));
    presentedRoot->addSubview(presentedGlass);
    auto presentedController = new_shared<UIViewController>();
    presentedController->setView(presentedRoot);
    presentedController->setModalPresentationStyle(
        UIModalPresentationStyle::overFullScreen
    );
    focusNavigationController->present(presentedController, false);

    const auto initialLightTint = glassTint(presentedGlass);
    const auto initialLightLuminosity = glassScalar(
        presentedGlass,
        "luminosity"
    );
    expect(
        initialLightTint[0] > 0.99f
            && initialLightTint[1] > 0.99f
            && initialLightTint[2] > 0.99f
            && initialLightTint[3] < 0.30f
            && initialLightLuminosity > 0.10f,
        "presented regular glass starts with the translucent light material"
    );

    skiaContext->themeMode = UIUserInterfaceStyle::dark;
    expect(applicationRunLoop(), "the dark trait update completes");
    expect(
        presentationChrome->traitChangeCount == 1
            && presentationChrome->traitCollection()
            && presentationChrome->traitCollection()->userInterfaceStyle()
                == UIUserInterfaceStyle::dark,
        "window-owned presentation chrome receives the dark trait update"
    );
    const auto darkTint = glassTint(presentedGlass);
    const auto darkLuminosity = glassScalar(presentedGlass, "luminosity");
    expect(
        darkTint[0] < 0.01f
            && darkTint[1] < 0.01f
            && darkTint[2] < 0.01f
            && darkTint[3] > initialLightTint[3]
            && darkLuminosity < 0.001f,
        "an active presented glass view reinstalls its dark material"
    );

    skiaContext->themeMode = UIUserInterfaceStyle::light;
    expect(applicationRunLoop(), "the restored light trait update completes");
    const auto restoredLightTint = glassTint(presentedGlass);
    expect(
        restoredLightTint == initialLightTint
            && glassScalar(presentedGlass, "luminosity")
                == initialLightLuminosity,
        "an active presented glass view restores its light material"
    );
    presentedController->dismiss(false);

    const bool eventSubsystemReady = SDL_InitSubSystem(SDL_INIT_EVENTS);
    expect(eventSubsystemReady, "SDL's event subsystem is available for tab focus routing");
    if (eventSubsystemReady) {
        expect(
            sendKeyPress(application, SDLK_RETURN, SDL_SCANCODE_RETURN),
            "the A/select key press is queued"
        );
        focusNavigationController->defaultActionsWidget()->refresh();
        expect(
            hierarchyHasTraitCollection(focusNavigationController->defaultActionsWidget()),
            "a focus-triggered action-legend rebuild inherits the window traits"
        );
        expect(
            focusWindow->focusSystem()->focusedItem().lock() == focusableController->control,
            "a tab item's A action moves focus into its presented controller"
        );

        const auto focusGainCount = focusableController->control->focusGainCount;
        const auto focusLossCount = focusableController->control->focusLossCount;
        const auto focusMovementFailureCount =
            focusableController->control->focusMovementFailureCount;
        focusWindow->sendEvent(new_shared<UIEvent>());
        expect(
            focusWindow->focusSystem()->focusedItem().expired(),
            "touch input deactivates visible controller focus"
        );
        expect(
            focusableController->control->focusLossCount == focusLossCount + 1,
            "touch input runs the focused control's loss animation"
        );
        expect(
            sendKeyPress(application, SDLK_RETURN, SDL_SCANCODE_RETURN),
            "controller input can reactivate the retained focused item"
        );
        expect(
            focusWindow->focusSystem()->focusedItem().lock() == focusableController->control,
            "controller input restores the retained focused item"
        );
        expect(
            focusableController->control->focusGainCount == focusGainCount + 1,
            "reactivation runs the retained control's gain animation again"
        );
        expect(
            focusableController->control->focusMovementFailureCount
                == focusMovementFailureCount,
            "reactivation is not reported as a failed same-item focus move"
        );

        expect(
            sendKeyPress(application, SDLK_ESCAPE, SDL_SCANCODE_ESCAPE),
            "the B/menu key press is queued"
        );
        focusNavigationController->defaultActionsWidget()->refresh();
        focusedView = std::dynamic_pointer_cast<UIView>(
            focusWindow->focusSystem()->focusedItem().lock()
        );
        expect(
            focusedView && focusedView->isDescendantOf(focusTabController->tabBar()),
            "an unhandled B action returns focus to the selected tab item"
        );

        expect(
            sendKeyPress(application, SDLK_RETURN, SDL_SCANCODE_RETURN),
            "A can enter the presented controller again"
        );
        focusNavigationController->defaultActionsWidget()->refresh();
        focusableController->control->consumesBack = true;
        expect(
            sendKeyPress(application, SDLK_ESCAPE, SDL_SCANCODE_ESCAPE),
            "the child-handled B/menu key press is queued"
        );
        focusNavigationController->defaultActionsWidget()->refresh();
        expect(
            focusWindow->focusSystem()->focusedItem().lock() == focusableController->control,
            "a child responder can consume B before it reaches the tab controller"
        );
        expect(
            focusableController->control->consumedBackCount == 1,
            "the child responder receives the B action once"
        );
        focusableController->control->consumesBack = false;
        expect(
            sendKeyPress(application, SDLK_ESCAPE, SDL_SCANCODE_ESCAPE),
            "B can return from tab content before exiting"
        );
        focusNavigationController->defaultActionsWidget()->refresh();
        const auto& topLevelActions =
            focusNavigationController->defaultActionsWidget()->actions();
        expect(
            std::any_of(
                topLevelActions.begin(),
                topLevelActions.end(),
                [](const auto& action) {
                    return action.button == NXActionButton::b
                        && action.isEnabled
                        && action.action.title() == "Exit";
                }
            ),
            "the tab bar exposes navigation Exit once there is nowhere farther back"
        );

        focusWindow->sendEvent(new_shared<UIEvent>());
        expect(
            focusTabController->tabBar()->activateItemAt(IndexPath(1, 0)),
            "touch-mode user selection can activate a different tab"
        );
        expect(
            focusWindow->focusSystem()->requestFocusUpdate(otherFocusableController),
            "the touch action can retain focus inside the newly selected content"
        );
        expect(
            sendKeyPress(application, SDLK_LEFT, SDL_SCANCODE_LEFT),
            "the first controller direction reactivates retained focus"
        );
        expect(
            focusWindow->focusSystem()->focusedItem().lock()
                == otherFocusableController->control,
            "controller focus resumes in the touch-selected tab content"
        );
        expect(
            sendKeyPress(application, SDLK_LEFT, SDL_SCANCODE_LEFT),
            "the second controller direction returns to the tab bar"
        );
        const auto touchSelectedTab = std::dynamic_pointer_cast<NXTabBarButton>(
            focusWindow->focusSystem()->focusedItem().lock()
        );
        expect(
            touchSelectedTab && touchSelectedTab->text() == "Other",
            "returning from touch-selected content focuses its selected tab item"
        );

        expect(
            sendKeyPress(application, SDLK_ESCAPE, SDL_SCANCODE_ESCAPE),
            "the top-level B/menu key press is queued"
        );
        expect(application->isQuitRequested(), "top-level B requests app exit");
        SDL_QuitSubSystem(SDL_INIT_EVENTS);
    }

    focusWindow.reset();
    applicationDelegate.reset();
    application.reset();
    UIApplication::shared.reset();

    if (failures == 0) {
        std::cout << "NXTabBarController tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
