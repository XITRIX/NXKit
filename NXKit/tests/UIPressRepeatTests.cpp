#include <UIApplication.h>
#include <UIApplicationDelegate.h>
#include <UIControl.h>
#include <UIPress.h>
#include <UIResponder.h>
#include <UIViewController.h>
#include <UIWindow.h>

#include <SDL3/SDL.h>

#include <algorithm>
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

    SDL_QuitSubSystem(SDL_INIT_EVENTS);
    UIApplication::shared.reset();

    if (failures == 0) {
        std::cout << "UIPress repeat tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
