#include <UIApplication.h>
#include <UIApplicationDelegate.h>
#include <UIControl.h>
#include <UIPress.h>
#include <UIPressesEvent.h>
#include <UIResponder.h>
#include <UIScrollView.h>
#include <UITextView.h>
#include <UIViewController.h>
#include <UIWindow.h>

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace NXKit;

namespace NXKit {

bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(
    UIApplication*,
    std::map<std::string, std::any>
) {
    return true;
}

class UIApplicationPressRepeatTestHarness {
public:
    static void sendRepeatsAt(
        const std::shared_ptr<UIApplication>& application,
        Timer timestamp
    ) {
        application->sendPressRepeatsIfNeeded(timestamp);
    }

    static void handleSDLEvent(
        const std::shared_ptr<UIApplication>& application,
        SDL_Event event
    ) {
        application->handleSDLEvent(event);
    }

    static size_t activePressCount() {
        return UIPressesEvent::activePressesEvents.size();
    }

    static UIPressType firstActivePressType() {
        if (UIPressesEvent::activePressesEvents.empty()) {
            return UIPressType::none;
        }
        const auto& event = UIPressesEvent::activePressesEvents.front();
        if (!event || event->allPresses().empty()) {
            return UIPressType::none;
        }
        return (*event->allPresses().begin())->type();
    }
};

class UIScrollViewFocusTestHarness {
public:
    static void advanceNaturalScroll(
        const std::shared_ptr<UIScrollView>& scrollView,
        double elapsedSeconds
    ) {
        scrollView->advanceNaturalFocusScroll(elapsedSeconds);
    }

    static bool naturalScrollIsActive(
        const std::shared_ptr<UIScrollView>& scrollView
    ) {
        return scrollView->_naturalFocusScrollActive;
    }
};

} // namespace NXKit

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (condition) {
        return;
    }
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}

bool nearlyEqual(NXFloat lhs, NXFloat rhs) {
    return std::abs(lhs - rhs) < 0.01f;
}

bool queueKeyEvent(
    SDL_EventType type,
    SDL_Keycode keycode,
    SDL_Scancode scancode,
    bool isPlatformRepeat = false
) {
    SDL_Event event {};
    event.type = type;
    event.key.key = keycode;
    event.key.scancode = scancode;
    event.key.down = type == SDL_EVENT_KEY_DOWN;
    event.key.repeat = isPlatformRepeat;
    return SDL_PushEvent(&event);
}

class DirectionalPressConsumingControl final : public UIControl {
public:
    void pressesBegan(
        std::set<std::shared_ptr<UIPress>>,
        std::shared_ptr<UIPressesEvent>
    ) override {}
};

} // namespace

int main() {
    UIAction defaultAction;
    expect(
        defaultAction.repeatBehavior()
            == UIMenuElementRepeatBehavior::automatic,
        "UIAction uses UIKit's automatic repeat policy by default"
    );

    const bool eventSubsystemReady = SDL_InitSubSystem(SDL_INIT_EVENTS);
    expect(eventSubsystemReady, "SDL's event subsystem is available for repeat tests");
    if (!eventSubsystemReady) {
        return 1;
    }

    auto application = std::make_shared<UIApplication>();
    auto applicationDelegate = std::make_shared<UIApplicationDelegate>();
    auto window = new_shared<UIWindow>();
    auto rootController = new_shared<UIViewController>();
    window->setFrame(NXRect(0, 0, 1280, 720));
    window->setRootViewController(rootController);
    rootController->view()->setFrame(window->bounds());
    rootController->view()->configureLayout(
        [](const std::shared_ptr<YGLayout>& layout) {
            layout->setFlexDirection(YGFlexDirectionRow);
        }
    );

    auto first = new_shared<UIControl>();
    auto second = new_shared<UIControl>();
    auto third = new_shared<UIControl>();
    rootController->view()->addSubview(first);
    rootController->view()->addSubview(second);
    rootController->view()->addSubview(third);
    window->addSubview(rootController->view());
    window->updateFocus();

    applicationDelegate->window = window;
    application->delegate = applicationDelegate;
    application->keyWindow = window;
    UIApplication::shared = application;

    expect(
        window->focusSystem()->focusedItem().lock() == first,
        "repeat test starts on the first focusable item"
    );
    expect(
        queueKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT, SDL_SCANCODE_RIGHT),
        "initial directional key-down is queued"
    );
    application->handleEventsIfNeeded();
    expect(
        window->focusSystem()->focusedItem().lock() == second,
        "a regular directional press moves focus once"
    );

    expect(
        queueKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT, SDL_SCANCODE_RIGHT),
        "a duplicate physical directional key-down is queued"
    );
    application->handleEventsIfNeeded();
    expect(
        window->focusSystem()->focusedItem().lock() == second
            && UIApplicationPressRepeatTestHarness::activePressCount() == 1,
        "duplicate physical key-down events do not create a stuck repeat"
    );

    expect(
        queueKeyEvent(
            SDL_EVENT_KEY_DOWN,
            SDLK_RIGHT,
            SDL_SCANCODE_RIGHT,
            true
        ),
        "platform keyboard repeat is queued"
    );
    application->handleEventsIfNeeded();
    expect(
        window->focusSystem()->focusedItem().lock() == second,
        "platform-native repeats are ignored in favor of NXKit timing"
    );

    UIApplicationPressRepeatTestHarness::sendRepeatsAt(
        application,
        Timer(240)
    );
    expect(
        window->focusSystem()->focusedItem().lock() == second,
        "holding for less than 250 ms does not repeat"
    );
    UIApplicationPressRepeatTestHarness::sendRepeatsAt(
        application,
        Timer(260)
    );
    expect(
        window->focusSystem()->focusedItem().lock() == third,
        "the first repeat moves focus after Borealis's 250 ms threshold"
    );
    expect(
        queueKeyEvent(SDL_EVENT_KEY_UP, SDLK_RIGHT, SDL_SCANCODE_RIGHT),
        "directional key-up is queued"
    );
    application->handleEventsIfNeeded();

    int regularActionCount = 0;
    int repeatingActionCount = 0;
    std::vector<bool> observedRepeatFlags;

    UIAction regularAction("Regular", [&regularActionCount]() {
        ++regularActionCount;
    });
    UIAction repeatingAction("Repeating", [&repeatingActionCount]() {
        ++repeatingActionCount;
    });
    repeatingAction.setRepeatBehavior(
        UIMenuElementRepeatBehavior::repeatable
    );

    const auto menuMatcher = [&observedRepeatFlags](
        const std::shared_ptr<UIPress>& press
    ) {
        if (!press || press->type() != UIPressType::menu) {
            return false;
        }
        observedRepeatFlags.push_back(press->isRepeat());
        return true;
    };
    third->registerAction(UIResponderAction {
        .identifier = "NXKit.tests.pressRepeat.regular",
        .inputIdentifier = UIResponderActionInputMenu,
        .action = regularAction,
        .matches = menuMatcher,
        .priority = 10,
    });
    third->registerAction(UIResponderAction {
        .identifier = "NXKit.tests.pressRepeat.repeating",
        .inputIdentifier = UIResponderActionInputMenu,
        .action = repeatingAction,
        .matches = menuMatcher,
    });

    expect(
        queueKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_ESCAPE, SDL_SCANCODE_ESCAPE),
        "action key-down is queued"
    );
    application->handleEventsIfNeeded();
    UIApplicationPressRepeatTestHarness::sendRepeatsAt(
        application,
        Timer(260)
    );
    UIApplicationPressRepeatTestHarness::sendRepeatsAt(
        application,
        Timer(370)
    );
    expect(
        regularActionCount == 0 && repeatingActionCount == 2,
        "only an explicitly repeatable action receives held-button deliveries"
    );
    expect(
        queueKeyEvent(SDL_EVENT_KEY_UP, SDLK_ESCAPE, SDL_SCANCODE_ESCAPE),
        "action key-up is queued"
    );
    application->handleEventsIfNeeded();
    expect(
        regularActionCount == 1 && repeatingActionCount == 2,
        "the higher-priority regular action still receives the physical click"
    );
    expect(
        std::find(
            observedRepeatFlags.begin(),
            observedRepeatFlags.end(),
            true
        ) != observedRepeatFlags.end()
            && std::find(
                observedRepeatFlags.begin(),
                observedRepeatFlags.end(),
                false
            ) != observedRepeatFlags.end(),
        "UIPress distinguishes repeat deliveries from the regular release"
    );

    int primaryActionCount = 0;
    UIAction primaryAction("Primary", [&primaryActionCount]() {
        ++primaryActionCount;
    });
    primaryAction.setRepeatBehavior(
        UIMenuElementRepeatBehavior::repeatable
    );
    third->primaryAction = primaryAction;

    expect(
        queueKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN, SDL_SCANCODE_RETURN),
        "repeatable control key-down is queued"
    );
    application->handleEventsIfNeeded();
    UIApplicationPressRepeatTestHarness::sendRepeatsAt(
        application,
        Timer(260)
    );
    UIApplicationPressRepeatTestHarness::sendRepeatsAt(
        application,
        Timer(370)
    );
    expect(
        primaryActionCount == 2 && third->isHighlighted(),
        "a repeatable primary action clicks repeatedly without losing hold highlight"
    );
    expect(
        queueKeyEvent(SDL_EVENT_KEY_UP, SDLK_RETURN, SDL_SCANCODE_RETURN),
        "repeatable control key-up is queued"
    );
    application->handleEventsIfNeeded();
    expect(
        primaryActionCount == 3 && !third->isHighlighted(),
        "the physical release remains a regular click and ends highlighting"
    );

    SDL_Event axisEvent {};
    axisEvent.type = SDL_EVENT_GAMEPAD_AXIS_MOTION;
    axisEvent.gaxis.which = 41;
    axisEvent.gaxis.axis = SDL_GAMEPAD_AXIS_LEFTY;
    axisEvent.gaxis.value = 32767;
    UIApplicationPressRepeatTestHarness::handleSDLEvent(
        application,
        axisEvent
    );
    expect(
        UIApplicationPressRepeatTestHarness::activePressCount() == 1,
        "a pressed thumbstick direction owns one repeat source"
    );
    axisEvent.gaxis.value = -32768;
    UIApplicationPressRepeatTestHarness::handleSDLEvent(
        application,
        axisEvent
    );
    expect(
        UIApplicationPressRepeatTestHarness::activePressCount() == 1
            && UIApplicationPressRepeatTestHarness::firstActivePressType()
                == UIPressType::upArrow,
        "reversing a thumbstick ends the old direction before beginning the new one"
    );
    axisEvent.gaxis.value = 0;
    UIApplicationPressRepeatTestHarness::handleSDLEvent(
        application,
        axisEvent
    );
    expect(
        UIApplicationPressRepeatTestHarness::activePressCount() == 0,
        "a thumbstick returning directly to neutral ends its repeat source"
    );

    SDL_Event gamepadButtonEvent {};
    gamepadButtonEvent.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
    gamepadButtonEvent.gbutton.which = 42;
    gamepadButtonEvent.gbutton.button = SDL_GAMEPAD_BUTTON_DPAD_DOWN;
    gamepadButtonEvent.gbutton.down = true;
    UIApplicationPressRepeatTestHarness::handleSDLEvent(
        application,
        gamepadButtonEvent
    );
    UIApplicationPressRepeatTestHarness::handleSDLEvent(
        application,
        gamepadButtonEvent
    );
    expect(
        UIApplicationPressRepeatTestHarness::activePressCount() == 1,
        "duplicate gamepad button-down events share one repeat source"
    );
    SDL_Event gamepadRemovedEvent {};
    gamepadRemovedEvent.type = SDL_EVENT_GAMEPAD_REMOVED;
    gamepadRemovedEvent.gdevice.which = 42;
    UIApplicationPressRepeatTestHarness::handleSDLEvent(
        application,
        gamepadRemovedEvent
    );
    expect(
        UIApplicationPressRepeatTestHarness::activePressCount() == 0,
        "disconnecting a gamepad cancels its held repeat sources"
    );

    auto focusScrollWindow = new_shared<UIWindow>();
    auto focusScrollRoot = new_shared<UIViewController>();
    focusScrollWindow->setFrame(NXRect(0, 0, 320, 200));
    focusScrollWindow->setRootViewController(focusScrollRoot);
    focusScrollRoot->view()->setFrame(focusScrollWindow->bounds());
    focusScrollWindow->addSubview(focusScrollRoot->view());

    auto focusScrollView = new_shared<UIScrollView>(NXRect(0, 0, 320, 200));
    focusScrollView->setContentSize(NXSize(320, 600));
    focusScrollRoot->view()->addSubview(focusScrollView);

    auto focusScrollContent = new_shared<UIView>(NXRect(0, 0, 320, 600));
    focusScrollContent->configureLayout(
        [](const std::shared_ptr<YGLayout>& layout) {
            layout->setFlexDirection(YGFlexDirectionColumn);
        }
    );
    focusScrollView->addSubview(focusScrollContent);

    auto naturalFirst = new_shared<UIControl>();
    naturalFirst->setFrame(NXRect(0, 0, 320, 60));
    focusScrollContent->addSubview(naturalFirst);
    auto naturalSecond = new_shared<DirectionalPressConsumingControl>();
    naturalSecond->setFrame(NXRect(0, 100, 320, 60));
    focusScrollContent->addSubview(naturalSecond);
    auto naturalThird = new_shared<UIControl>();
    naturalThird->setFrame(NXRect(0, 260, 320, 60));
    focusScrollContent->addSubview(naturalThird);
    auto naturalLast = new_shared<UIControl>();
    naturalLast->setFrame(NXRect(0, 540, 320, 60));
    focusScrollContent->addSubview(naturalLast);

    applicationDelegate->window = focusScrollWindow;
    application->keyWindow = focusScrollWindow;
    focusScrollWindow->updateFocus();
    expect(
        focusScrollWindow->focusSystem()->focusedItem().lock() == naturalFirst,
        "a natural scroll view initially focuses its first fully visible item"
    );

    auto safeAreaWindow = new_shared<UIWindow>();
    auto safeAreaRoot = new_shared<UIViewController>();
    safeAreaWindow->setFrame(NXRect(0, 0, 320, 200));
    safeAreaWindow->setRootViewController(safeAreaRoot);
    safeAreaRoot->view()->setFrame(safeAreaWindow->bounds());
    safeAreaRoot->setAdditionalSafeAreaInsets(UIEdgeInsets(0, 0, 40, 0));
    safeAreaWindow->addSubview(safeAreaRoot->view());

    auto safeAreaScrollView = new_shared<UIScrollView>(
        NXRect(0, 0, 320, 200)
    );
    safeAreaScrollView->setContentInsetAdjustmentBehavior(
        UIScrollViewContentInsetAdjustmentBehavior::never
    );
    safeAreaScrollView->setContentSize(NXSize(320, 400));
    safeAreaRoot->view()->addSubview(safeAreaScrollView);

    auto safeAreaContent = new_shared<UIView>(NXRect(0, 0, 320, 400));
    safeAreaScrollView->addSubview(safeAreaContent);
    auto safeAreaFirst = new_shared<UIControl>();
    safeAreaFirst->setFrame(NXRect(0, 0, 320, 60));
    safeAreaContent->addSubview(safeAreaFirst);
    auto safeAreaSecond = new_shared<UIControl>();
    safeAreaSecond->setFrame(NXRect(0, 120, 320, 60));
    safeAreaContent->addSubview(safeAreaSecond);

    applicationDelegate->window = safeAreaWindow;
    application->keyWindow = safeAreaWindow;
    safeAreaWindow->drawAndLayoutTreeIfNeeded();
    safeAreaWindow->updateFocus();
    expect(
        safeAreaScrollView->safeAreaInsets().bottom == 40
            && safeAreaWindow->focusSystem()->focusedItem().lock()
                == safeAreaFirst,
        "natural mode starts with a focus item inside the safe-area viewport"
    );
    expect(
        queueKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN, SDL_SCANCODE_DOWN),
        "the safe-area natural-mode down press is queued"
    );
    application->handleEventsIfNeeded();
    expect(
        safeAreaWindow->focusSystem()->focusedItem().lock() == safeAreaFirst
            && UIScrollViewFocusTestHarness::naturalScrollIsActive(
                safeAreaScrollView
            ),
        "natural mode scrolls before focusing an item obscured by a safe area"
    );
    UIScrollViewFocusTestHarness::advanceNaturalScroll(
        safeAreaScrollView,
        0.02
    );
    expect(
        nearlyEqual(safeAreaScrollView->contentOffset().y, 20)
            && safeAreaWindow->focusSystem()->focusedItem().lock()
                == safeAreaSecond,
        "natural mode transfers focus when the item clears the safe area"
    );
    expect(
        queueKeyEvent(SDL_EVENT_KEY_UP, SDLK_DOWN, SDL_SCANCODE_DOWN),
        "the safe-area natural-mode down release is queued"
    );
    application->handleEventsIfNeeded();

    applicationDelegate->window = focusScrollWindow;
    application->keyWindow = focusScrollWindow;
    expect(
        focusScrollWindow->focusSystem()->requestFocusUpdate(naturalLast)
            && focusScrollWindow->focusSystem()->focusedItem().lock()
                == naturalLast,
        "an exact descendant can receive programmatic focus"
    );
    expect(
        focusScrollWindow->focusSystem()->requestFocusUpdate(focusScrollView)
            && focusScrollWindow->focusSystem()->focusedItem().lock()
                == naturalFirst,
        "a direct scroll-view focus request resolves to its visible preferred child"
    );
    expect(
        focusScrollWindow->focusSystem()->requestFocusUpdate(focusScrollRoot)
            && focusScrollWindow->focusSystem()->focusedItem().lock()
                == naturalFirst,
        "a controller focus request descends through its scroll-view root"
    );

    expect(
        queueKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN, SDL_SCANCODE_DOWN),
        "the first natural-mode down press is queued"
    );
    application->handleEventsIfNeeded();
    expect(
        focusScrollWindow->focusSystem()->focusedItem().lock() == naturalSecond
            && focusScrollView->contentOffset() == NXPoint(),
        "natural mode visits a fully visible item before scrolling"
    );
    expect(
        queueKeyEvent(SDL_EVENT_KEY_UP, SDLK_DOWN, SDL_SCANCODE_DOWN),
        "the first natural-mode down release is queued"
    );
    application->handleEventsIfNeeded();

    expect(
        queueKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN, SDL_SCANCODE_DOWN),
        "the natural-mode scrolling press is queued"
    );
    application->handleEventsIfNeeded();
    expect(
        focusScrollWindow->focusSystem()->focusedItem().lock() == naturalSecond
            && focusScrollView->contentOffset() == NXPoint(),
        "natural mode retains focus while the incoming item is clipped"
    );
    UIScrollViewFocusTestHarness::advanceNaturalScroll(focusScrollView, 0.05);
    expect(
        nearlyEqual(focusScrollView->contentOffset().y, 50)
            && focusScrollWindow->focusSystem()->focusedItem().lock()
                == naturalSecond,
        "natural mode scrolls at a constant 1000 points per second"
    );
    UIScrollViewFocusTestHarness::advanceNaturalScroll(focusScrollView, 0.06);
    expect(
        nearlyEqual(focusScrollView->contentOffset().y, 110)
            && focusScrollWindow->focusSystem()->focusedItem().lock()
                == focusScrollView,
        "natural mode can temporarily focus the scroll view while both items are clipped"
    );
    UIScrollViewFocusTestHarness::advanceNaturalScroll(focusScrollView, 0.01);
    expect(
        nearlyEqual(focusScrollView->contentOffset().y, 120)
            && focusScrollWindow->focusSystem()->focusedItem().lock()
                == naturalThird,
        "natural mode hands focus off exactly when the incoming item is fully visible"
    );
    expect(
        queueKeyEvent(SDL_EVENT_KEY_UP, SDLK_DOWN, SDL_SCANCODE_DOWN),
        "the natural-mode scrolling release is queued"
    );
    naturalSecond->removeFromSuperview();
    application->handleEventsIfNeeded();
    const auto releasedOffset = focusScrollView->contentOffset();
    UIScrollViewFocusTestHarness::advanceNaturalScroll(focusScrollView, 0.05);
    expect(
        focusScrollView->contentOffset() == releasedOffset,
        "natural scrolling stops even when the original press responder was removed"
    );

    auto naturalVisibleAtOffset = new_shared<UIControl>();
    naturalVisibleAtOffset->setFrame(NXRect(0, 160, 320, 40));
    focusScrollContent->addSubview(naturalVisibleAtOffset);

    focusScrollView->setFocusTrackingMode(
        UIScrollViewFocusTrackingMode::centered
    );
    focusScrollView->setContentOffset(NXPoint(), false);
    expect(
        focusScrollWindow->focusSystem()->requestFocusUpdate(naturalThird)
            && nearlyEqual(focusScrollView->contentOffset().y, 190),
        "centered mode centers a focused item when the position is reachable"
    );
    expect(
        focusScrollWindow->focusSystem()->requestFocusUpdate(naturalLast)
            && nearlyEqual(focusScrollView->contentOffset().y, 400),
        "centered mode clamps edge items to the natural scroll limit"
    );

    focusScrollView->setFocusTrackingMode(
        UIScrollViewFocusTrackingMode::focused
    );
    focusScrollView->setContentOffset(NXPoint(), false);
    expect(
        focusScrollWindow->focusSystem()->requestFocusUpdate(naturalThird)
            && nearlyEqual(focusScrollView->contentOffset().y, 120),
        "focused mode performs the minimum scroll needed to reveal an item"
    );
    expect(
        focusScrollWindow->focusSystem()->requestFocusUpdate(
            naturalVisibleAtOffset
        ) && nearlyEqual(focusScrollView->contentOffset().y, 120),
        "focused mode does not scroll when the new item is already fully visible"
    );

    auto textWindow = new_shared<UIWindow>();
    auto textRoot = new_shared<UIViewController>();
    textWindow->setFrame(NXRect(0, 0, 320, 200));
    textWindow->setRootViewController(textRoot);
    textRoot->view()->setFrame(textWindow->bounds());
    textWindow->addSubview(textRoot->view());
    auto textScrollView = new_shared<UITextView>(NXRect(0, 0, 320, 200));
    textScrollView->setContentSize(NXSize(320, 600));
    textRoot->view()->addSubview(textScrollView);

    applicationDelegate->window = textWindow;
    application->keyWindow = textWindow;
    textWindow->updateFocus();
    expect(
        textWindow->focusSystem()->focusedItem().lock() == textScrollView,
        "a text view with no focusable children can itself receive focus"
    );
    expect(
        queueKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN, SDL_SCANCODE_DOWN),
        "the focused text-view scroll press is queued"
    );
    application->handleEventsIfNeeded();
    UIScrollViewFocusTestHarness::advanceNaturalScroll(textScrollView, 0.1);
    expect(
        nearlyEqual(textScrollView->contentOffset().y, 100),
        "a focused text view scrolls long content with directional input"
    );
    expect(
        queueKeyEvent(SDL_EVENT_KEY_UP, SDLK_DOWN, SDL_SCANCODE_DOWN),
        "the focused text-view scroll release is queued"
    );
    application->handleEventsIfNeeded();
    UIScrollViewFocusTestHarness::advanceNaturalScroll(textScrollView, 0.01);
    expect(
        !UIScrollViewFocusTestHarness::naturalScrollIsActive(textScrollView),
        "releasing a text-view direction invalidates its display-link scroll"
    );
    expect(
        queueKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_UP, SDL_SCANCODE_UP),
        "a subsequent focused text-view scroll press is queued"
    );
    application->handleEventsIfNeeded();
    expect(
        UIScrollViewFocusTestHarness::naturalScrollIsActive(textScrollView),
        "a text view starts a fresh display-link scroll after its prior one stopped"
    );
    UIScrollViewFocusTestHarness::advanceNaturalScroll(textScrollView, 0.04);
    expect(
        nearlyEqual(textScrollView->contentOffset().y, 60),
        "a text view keeps responding after the first controller scroll"
    );
    expect(
        queueKeyEvent(SDL_EVENT_KEY_UP, SDLK_UP, SDL_SCANCODE_UP),
        "the subsequent focused text-view scroll release is queued"
    );
    application->handleEventsIfNeeded();

    SDL_QuitSubSystem(SDL_INIT_EVENTS);
    UIApplication::shared.reset();

    if (failures == 0) {
        std::cout << "UIPress repeat tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
