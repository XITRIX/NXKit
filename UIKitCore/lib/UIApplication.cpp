#include <UIApplication.h>
#include <UITouch.h>
#include <UIPress.h>
#include <UIPressesEvent.h>
#include <SDL3/SDL.h>
#include <SkiaCtx.h>

#include <cmath>

using namespace NXKit;

std::shared_ptr<UIApplication> UIApplication::shared = nullptr;

UIApplication::UIApplication() {
//    // TODO: Replace with Bunbles
//#ifdef USE_LIBROMFS
//    Utils::resourcePath = "";
//#elif __SWITCH__
//    Utils::resourcePath = "romfs:/";
//#elif SDL_PLATFORM_APPLE
//#include <TargetConditionals.h>
//#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
//    Utils::resourcePath = std::string(SDL_GetBasePath()) + "/assets/";
//#endif
//#endif

//    UIFont.loadSystemFonts();
}

std::map<SDL_JoystickID, SDL_Gamepad*> gamepads;

UIApplication::~UIApplication() {
    if (lifecycleEventWatchInstalled) {
        SDL_RemoveEventWatch(handleSDLLifecycleEvent, this);
    }

    for (const auto& [id, gamepad] : gamepads) {
        SDL_CloseGamepad(gamepad);
    }
    gamepads.clear();
}

UIGamepadKey UIApplication::mapGamepadButtonEventToUIGamepadKey(SDL_GamepadButtonEvent event) {
    UIGamepadKey key;
    key._value = event.down ? 1.0f : 0.0f;

    switch (event.button) {
        case SDL_GAMEPAD_BUTTON_DPAD_UP: {
            key._inputType = UIGamepadInputType::up;
            break;
        }
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN: {
            key._inputType = UIGamepadInputType::down;
            break;
        }
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: {
            key._inputType = UIGamepadInputType::right;
            break;
        }
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT: {
            key._inputType = UIGamepadInputType::left;
            break;
        }
        case SDL_GAMEPAD_BUTTON_SOUTH: {
            key._inputType = UIGamepadInputType::buttonA;
            break;
        }
        case SDL_GAMEPAD_BUTTON_EAST: {
            key._inputType = UIGamepadInputType::buttonB;
            break;
        }
        case SDL_GAMEPAD_BUTTON_WEST: {
            key._inputType = UIGamepadInputType::buttonX;
            break;
        }
        case SDL_GAMEPAD_BUTTON_NORTH: {
            key._inputType = UIGamepadInputType::buttonY;
            break;
        }
        case SDL_GAMEPAD_BUTTON_START: {
            key._inputType = UIGamepadInputType::buttonStart;
            break;
        }
        case SDL_GAMEPAD_BUTTON_BACK: {
            key._inputType = UIGamepadInputType::buttonOptions;
            break;
        }
        case SDL_GAMEPAD_BUTTON_GUIDE: {
            key._inputType = UIGamepadInputType::buttonGuide;
            break;
        }
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: {
            key._inputType = UIGamepadInputType::leftShoulder;
            break;
        }
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: {
            key._inputType = UIGamepadInputType::rightShoulder;
            break;
        }
        case SDL_GAMEPAD_BUTTON_LEFT_STICK: {
            key._inputType = UIGamepadInputType::leftThumbstickButton;
            break;
        }
        case SDL_GAMEPAD_BUTTON_RIGHT_STICK: {
            key._inputType = UIGamepadInputType::rightThumbstickButton;
            break;
        }
        default: break;
    }

    return key;
}

std::optional<UIGamepadKey> UIApplication::mapGamepadAxisEventToUIGamepadKey(SDL_GamepadAxisEvent event) {
    UIGamepadKey key;
    key._value = float(event.value) / 32767.0f;

    switch (event.axis) {
        case SDL_GAMEPAD_AXIS_LEFTX: {
            if (key._value < 0) {
                key._inputType = UIGamepadInputType::leftThumbstickAxisLeft;
                break;
            }
            if (key._value > 0) {
                key._inputType = UIGamepadInputType::leftThumbstickAxisRight;
                break;
            }
            return std::nullopt;
        }
        case SDL_GAMEPAD_AXIS_LEFTY: {
            if (key._value < 0) {
                key._inputType = UIGamepadInputType::leftThumbstickAxisUp;
                break;
            }
            if (key._value > 0) {
                key._inputType = UIGamepadInputType::leftThumbstickAxisDown;
                break;
            }
            return std::nullopt;
        }
        case SDL_GAMEPAD_AXIS_RIGHTX: {
            if (key._value < 0) {
                key._inputType = UIGamepadInputType::rightThumbstickAxisLeft;
                break;
            }
            if (key._value > 0) {
                key._inputType = UIGamepadInputType::rightThumbstickAxisRight;
                break;
            }
            return std::nullopt;
        }
        case SDL_GAMEPAD_AXIS_RIGHTY: {
            if (key._value < 0) {
                key._inputType = UIGamepadInputType::rightThumbstickAxisUp;
                break;
            }
            if (key._value > 0) {
                key._inputType = UIGamepadInputType::rightThumbstickAxisDown;
                break;
            }
            return std::nullopt;
        }
        case SDL_GAMEPAD_AXIS_LEFT_TRIGGER: {
            key._inputType = UIGamepadInputType::leftTrigger;
            break;
        }
        case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: {
            key._inputType = UIGamepadInputType::rightTrigger;
            break;
        }
    }

    key._value = std::abs(key._value);
    return key;
}

UIPressType UIApplication::mapGamepadInputToUIPressType(UIGamepadInputType key) {
    switch (key) {
        case UIGamepadInputType::up:
        case UIGamepadInputType::leftThumbstickAxisUp:
        case UIGamepadInputType::rightThumbstickAxisUp:
            return UIPressType::upArrow;

        case UIGamepadInputType::down:
        case UIGamepadInputType::leftThumbstickAxisDown:
        case UIGamepadInputType::rightThumbstickAxisDown:
            return UIPressType::downArrow;

        case UIGamepadInputType::left:
        case UIGamepadInputType::leftThumbstickAxisLeft:
        case UIGamepadInputType::rightThumbstickAxisLeft:
            return UIPressType::leftArrow;

        case UIGamepadInputType::right:
        case UIGamepadInputType::leftThumbstickAxisRight:
        case UIGamepadInputType::rightThumbstickAxisRight:
            return UIPressType::rightArrow;

#if defined(PLATFORM_SWITCH) // TODO: Add other cases when B can handle "select" action (Japanese localization i.e.)
        case UIGamepadInputType::buttonB:
            return UIPressType::select;
        case UIGamepadInputType::buttonA:
            return UIPressType::menu;
#else
        case UIGamepadInputType::buttonA:
            return UIPressType::select;
        case UIGamepadInputType::buttonB:
            return UIPressType::menu;
#endif

        default: return UIPressType::none;
    }
}

void UIApplication::handleEventsIfNeeded() {
    auto e = SDL_Event();

    while (SDL_PollEvent(&e)) {
        handleSDLEvent(e);
    }
}

void UIApplication::startHandlingLifecycleEvents() {
    if (lifecycleEventWatchInstalled) {
        return;
    }

    lifecycleEventWatchInstalled = SDL_AddEventWatch(handleSDLLifecycleEvent, this);
    if (!lifecycleEventWatchInstalled) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Could not register the SDL lifecycle event watch: %s",
                     SDL_GetError());
    }
}

bool SDLCALL UIApplication::handleSDLLifecycleEvent(void* userdata, SDL_Event* event) {
    auto* application = static_cast<UIApplication*>(userdata);
    if (!application || !event) {
        return true;
    }

    switch (event->type) {
        case SDL_EVENT_WILL_ENTER_BACKGROUND:
            onWillEnterBackground();
            break;
        case SDL_EVENT_DID_ENTER_BACKGROUND:
            onDidEnterBackground();
            break;
        case SDL_EVENT_WILL_ENTER_FOREGROUND:
            onWillEnterForeground();
            break;
        case SDL_EVENT_DID_ENTER_FOREGROUND:
            onDidEnterForeground();
            break;
        default:
            break;
    }

    return true;
}

void UIApplication::handleSDLEvent(SDL_Event e) {
    switch (e.type) {
        case SDL_EVENT_QUIT: {
            handleSDLQuit();
            return;
        }
        case SDL_EVENT_FINGER_DOWN: {
            const auto screenSize = SkiaCtx::main()->getSize();
            auto renderSize = screenSize;
            auto fingerPoint = NXPoint(renderSize.width * e.tfinger.x, renderSize.height * e.tfinger.y);
//                printf("Touch id: %llu Begin, X:%f - Y:%f\n", e.tfinger.fingerID, fingerPoint.x, fingerPoint.y);
            auto touch = new_shared<UITouch>(e.tfinger.fingerID, fingerPoint, Timer());
            auto event = std::shared_ptr<UIEvent>(new UIEvent(touch));
            UIEvent::activeEvents.push_back(event);
            sendEvent(event);
            break;
        }
        case SDL_EVENT_FINGER_MOTION: {
            const auto screenSize = SkiaCtx::main()->getSize();
            auto renderSize = screenSize;
            auto fingerPoint = NXPoint(renderSize.width * e.tfinger.x, renderSize.height * e.tfinger.y);
//                printf("Touch id: %llu Moved, X:%f - Y:%f\n", e.tfinger.fingerID, fingerPoint.x, fingerPoint.y);

            std::shared_ptr<UIEvent> event;
            std::shared_ptr<UITouch> touch;

            for (auto& levent: UIEvent::activeEvents) {
                for (auto& ltouch: levent->allTouches()) {
                    if (ltouch->touchId() == e.tfinger.fingerID) {
                        event = levent;
                        touch = ltouch;
                    }
                }
            }

            if (!event || !touch) return;

            auto previousTimestamp = touch->timestamp();
            auto newTimestamp = Timer();

            touch->updateAbsoluteLocation(fingerPoint);
            touch->_timestamp = newTimestamp;
            touch->_phase = UITouchPhase::moved;

            // Timer::operator- returns milliseconds.
            // SDL can emit back-to-back motion events with unrealistically tiny deltas, so drop samples below 5 ms.
            if ((newTimestamp - previousTimestamp) > 5.0) {
                sendEvent(event);
            }

            break;
        }
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_CANCELED: {
//                printf("Touch id: %llu Ended, X:%f - Y:%f\n", e.tfinger.fingerID, fingerPoint.x, fingerPoint.y);

            std::shared_ptr<UIEvent> event;
            std::shared_ptr<UITouch> touch;

            for (auto& levent: UIEvent::activeEvents) {
                for (auto& ltouch: levent->allTouches()) {
                    if (ltouch->touchId() == e.tfinger.fingerID) {
                        event = levent;
                        touch = ltouch;
                    }
                }
            }

            if (!event || !touch) return;

            touch->_timestamp = Timer();
            touch->_phase = e.type == SDL_EVENT_FINGER_CANCELED
                ? UITouchPhase::cancelled
                : UITouchPhase::ended;

            sendEvent(event);
            UIEvent::activeEvents.erase(std::remove(UIEvent::activeEvents.begin(), UIEvent::activeEvents.end(), event), UIEvent::activeEvents.end());

            break;
        }
#ifndef PLATFORM_IOS
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            const auto screenSize = SkiaCtx::main()->getSize();
            // Simulate touch
            auto touchEvent = SDL_Event();
            touchEvent.type = SDL_EVENT_FINGER_DOWN;

            auto renderSize = screenSize;
            touchEvent.tfinger.x = float(e.button.x) / renderSize.width;
            touchEvent.tfinger.y = float(e.button.y) / renderSize.height;
            touchEvent.tfinger.fingerID = static_cast<SDL_FingerID>(-1);

            handleSDLEvent(touchEvent);
            break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
            const auto screenSize = SkiaCtx::main()->getSize();
            // Simulate touch
            auto touchEvent = SDL_Event();
            touchEvent.type = SDL_EVENT_FINGER_MOTION;

            auto renderSize = screenSize;
            touchEvent.tfinger.x = e.motion.x / renderSize.width;
            touchEvent.tfinger.y = e.motion.y / renderSize.height;
            touchEvent.tfinger.fingerID = static_cast<SDL_FingerID>(-1);

            handleSDLEvent(touchEvent);
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            const auto screenSize = SkiaCtx::main()->getSize();
            // Simulate touch
            auto touchEvent = SDL_Event();
            touchEvent.type = SDL_EVENT_FINGER_UP;

            auto renderSize = screenSize;
            touchEvent.tfinger.x = float(e.button.x) / renderSize.width;
            touchEvent.tfinger.y = float(e.button.y) / renderSize.height;
            touchEvent.tfinger.fingerID = static_cast<SDL_FingerID>(-1);

            handleSDLEvent(touchEvent);
            break;
        }
#endif
        case SDL_EVENT_GAMEPAD_ADDED: {
            printf("Gamepad added\n");

            SDL_Gamepad* gamepad = SDL_OpenGamepad(e.gdevice.which);
            if (gamepad) {
                gamepads.emplace(e.gdevice.which, gamepad);
            } else {
                SDL_LogError(SDL_LOG_CATEGORY_INPUT,
                             "Could not open gamepad %u: %s",
                             e.gdevice.which,
                             SDL_GetError());
            }

            break;
        }
        case SDL_EVENT_GAMEPAD_REMOVED: {
            printf("Gamepad removed\n");
            const auto gamepad = gamepads.find(e.gdevice.which);
            if (gamepad != gamepads.end()) {
                SDL_CloseGamepad(gamepad->second);
                gamepads.erase(gamepad);
            }
            break;
        }
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
            printf("Gamepad button pressed\n");

            auto press = new_shared<UIPress>();
            press->_phase = UIPressPhase::began;
            press->setForWindow(delegate->window);
            press->_gamepadKey = mapGamepadButtonEventToUIGamepadKey(e.gbutton);
            press->_type = mapGamepadInputToUIPressType(press->_gamepadKey->inputType());

            auto event = std::shared_ptr<UIPressesEvent>(new UIPressesEvent(press));
            UIPressesEvent::activePressesEvents.push_back(event);
            sendEvent(event);

            break;
        }
        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            printf("Gamepad button released\n");

            std::shared_ptr<UIPressesEvent> event;
            std::shared_ptr<UIPress> press;

            UIGamepadKey gamepadKey = mapGamepadButtonEventToUIGamepadKey(e.gbutton);
            UIPressType type = mapGamepadInputToUIPressType(gamepadKey.inputType());

            for (auto& levent: UIPressesEvent::activePressesEvents) {
                for (auto& lpress: levent->allPresses()) {
                    if (type != UIPressType::none) {
                        if (lpress->type() == type) {
                            event = levent;
                            press = lpress;
                            goto FOUND;
                        }
                    } else if (lpress->gamepadKey().has_value()) {
                        if (lpress->gamepadKey().value().inputType() == gamepadKey.inputType()) {
                            lpress->_gamepadKey = gamepadKey;
                            event = levent;
                            press = lpress;
                            goto FOUND;
                        }
                    }
                }
            }
            FOUND:

            if (!event || !press) return;

            press->_timestamp = Timer();
            press->_phase = UIPressPhase::ended;

            sendEvent(event);
            UIPressesEvent::activePressesEvents.erase(std::remove(UIPressesEvent::activePressesEvents.begin(), UIPressesEvent::activePressesEvents.end(), event), UIPressesEvent::activePressesEvents.end());

            break;
        }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
            printf("Gamepad axis moved\n");

            static std::map<UIGamepadInputType, std::shared_ptr<UIPress>> _pressesMap;

            auto gamepadKey = mapGamepadAxisEventToUIGamepadKey(e.gaxis);
            if (!gamepadKey.has_value()) return;

            std::shared_ptr<UIPress> press;
            if (_pressesMap.find(gamepadKey->_inputType) == _pressesMap.end()) {
                press = new_shared<UIPress>();
                press->setForWindow(delegate->window);
                _pressesMap[gamepadKey->_inputType] = press;
                goto UPDATE_AND_SEND;
            } else {
                press = _pressesMap[gamepadKey->_inputType];
            }

            // Ignore to send the same isPressed value
            if (press->gamepadKey()->isPressed() == gamepadKey->isPressed()) return;

            UPDATE_AND_SEND:

            press->_phase = gamepadKey->isPressed() ? UIPressPhase::began : UIPressPhase::ended;
            press->_gamepadKey = gamepadKey;
            press->_type = mapGamepadInputToUIPressType(gamepadKey->inputType());

            auto event = std::shared_ptr<UIPressesEvent>(new UIPressesEvent(press));
            UIPressesEvent::activePressesEvents.push_back(event);
            sendEvent(event);

            break;
        }
        case SDL_EVENT_JOYSTICK_BUTTON_DOWN: {
            printf("Joystick button pressed\n");
            break;
        }
        case SDL_EVENT_JOYSTICK_BUTTON_UP: {
            printf("Joystick button released\n");
            break;
        }
        case SDL_EVENT_KEY_DOWN: {
            if (e.key.key == SDLK_Q) {
                handleSDLQuit();
            }

            auto press = new_shared<UIPress>();
            auto key = UIKey();
            key._keyCode = (UIKeyboardHIDUsage) e.key.scancode;
            key._modifierFlags = OptionSet<UIKeyModifierFlags>(e.key.mod);
            press->_key = key;
            press->_phase = UIPressPhase::began;
            press->setForWindow(delegate->window);

            switch (key._keyCode) {
                case UIKeyboardHIDUsage::keyboardRightArrow:
                    press->_type = UIPressType::rightArrow;
                    break;
                case UIKeyboardHIDUsage::keyboardUpArrow:
                    press->_type = UIPressType::upArrow;
                    break;
                case UIKeyboardHIDUsage::keyboardLeftArrow:
                    press->_type = UIPressType::leftArrow;
                    break;
                case UIKeyboardHIDUsage::keyboardDownArrow:
                    press->_type = UIPressType::downArrow;
                    break;
                case UIKeyboardHIDUsage::keyboardReturnOrEnter:
                    press->_type = UIPressType::select;
                    break;
                case UIKeyboardHIDUsage::keyboardEscape:
                    press->_type = UIPressType::menu;
                    break;
                default: break;
            }

            auto event = std::shared_ptr<UIPressesEvent>(new UIPressesEvent(press));
            UIPressesEvent::activePressesEvents.push_back(event);
            sendEvent(event);

            break;
        }
        case SDL_EVENT_KEY_UP: {
            std::shared_ptr<UIPressesEvent> event;
            std::shared_ptr<UIPress> press;

            for (auto& levent: UIPressesEvent::activePressesEvents) {
                for (auto& lpress: levent->allPresses()) {
                    if (!lpress->_key.has_value()) continue;

                    if ((int) lpress->_key->_keyCode == (int) e.key.scancode) {
                        event = levent;
                        press = lpress;
                    }
                }
            }

            if (!event || !press) return;

            press->_timestamp = Timer();
            press->_phase = UIPressPhase::ended;

            sendEvent(event);
            UIPressesEvent::activePressesEvents.erase(std::remove(UIPressesEvent::activePressesEvents.begin(), UIPressesEvent::activePressesEvents.end(), event), UIPressesEvent::activePressesEvents.end());

            break;

//#if DEBUG
//                let keyModifier = SDL_Keymod(UInt32(e.key.mod))
//                if keyModifier.contains(SDL_KMOD_LSHIFT) || keyModifier.contains(SDL_KMOD_RSHIFT) {
//                    switch e.key.key {
//                    case 43, 61: // +/*, +/= keys. TODO send key events via UIEvent
//                        break
//                    case 45: break // -/_ key
//                    case 118: // "V"
//                        keyWindow?.printViewHierarchy()
//                    default:
//                        print(e.key.key)
//                    }
//                }
//
//                if keyModifier.contains(SDL_KMOD_LGUI) || keyModifier.contains(SDL_KMOD_RGUI) {
//                    if e.key.key == 114 { // CMD-R
//                        UIScreen.main = nil
//                        UIScreen.main = UIScreen()
//                    }
//                }
//#endif
//
//                let scancode = e.key.scancode
//                if scancode == .androidHardwareBackButton || scancode == .escapeKey {
//                    keyWindow?.deepestPresentedView().handleHardwareBackButtonPress()
//                }
        }
        case SDL_EVENT_WILL_ENTER_BACKGROUND: {
            break;
        }
        case SDL_EVENT_DID_ENTER_BACKGROUND: {
            break;
        }
        case SDL_EVENT_WILL_ENTER_FOREGROUND: {
            break;
        }
        case SDL_EVENT_DID_ENTER_FOREGROUND: {
            break;
        }
        case SDL_EVENT_WINDOW_RESIZED: {
//                    UIRenderer::main()->refreshScreenResolution(e.window.data1, e.window.data2);
            break;
        }
        default:
            break;
    }
}

void UIApplication::handleSDLQuit() {
    UIEvent::activeEvents.clear();
    UIPressesEvent::activePressesEvents.clear();
    quitRequested = true;
}

 void UIApplication::sendEvent(const std::shared_ptr<UIEvent>& event) const {
     for (auto& touch: event->allTouches()) {
         touch->_window = keyWindow;
     }

     if (!keyWindow.expired())
         keyWindow.lock()->sendEvent(event);
 }

void UIApplication::onWillEnterForeground() {
    if (!UIApplication::shared || !UIApplication::shared->delegate) {
        return;
    }
    UIApplication::shared->delegate->applicationWillEnterForeground(UIApplication::shared.get());
//    UIApplication.post(willEnterForegroundNotification)
}

void UIApplication::onDidEnterForeground() {
    if (!UIApplication::shared || !UIApplication::shared->delegate) {
        return;
    }
    UIApplication::shared->delegate->applicationDidBecomeActive(UIApplication::shared.get());
//    UIApplication.post(didBecomeActiveNotification)
}

void UIApplication::onWillEnterBackground() {
    if (!UIApplication::shared || !UIApplication::shared->delegate) {
        return;
    }
    UIApplication::shared->delegate->applicationWillResignActive(UIApplication::shared.get());
//    UIApplication.post(willResignActiveNotification)
}

void UIApplication::onDidEnterBackground() {
    if (!UIApplication::shared || !UIApplication::shared->delegate) {
        return;
    }
    UIApplication::shared->delegate->applicationDidEnterBackground(UIApplication::shared.get());
//    UIApplication.post(didEnterBackgroundNotification)
}
