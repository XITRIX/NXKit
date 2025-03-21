#include <UIApplication.h>
#include <UITouch.h>
#include <UIPress.h>
#include <UIPressesEvent.h>
#include <SDL.h>
#include <SkiaCtx.h>

using namespace NXKit;

std::shared_ptr<UIApplication> UIApplication::shared = nullptr;

UIApplication::UIApplication() {
//    // TODO: Replace with Bunbles
//#ifdef USE_LIBROMFS
//    Utils::resourcePath = "";
//#elif __SWITCH__
//    Utils::resourcePath = "romfs:/";
//#elif __APPLE__
//#include <TargetConditionals.h>
//#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
//    Utils::resourcePath = std::string(SDL_GetBasePath()) + "/assets/";
//#endif
//#endif

//    UIFont.loadSystemFonts();
}

std::map<SDL_JoystickID, SDL_GameController*> controllers;

UIGamepadKey UIApplication::mapControllerButtonEventToUIGamepadKey(SDL_ControllerButtonEvent event) {
    UIGamepadKey key;
    key._value = event.state == SDL_PRESSED ? 1 : 0;

    switch (event.button) {
        case SDL_CONTROLLER_BUTTON_DPAD_UP: {
            key._inputType = UIGamepadInputType::up;
            break;
        }
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: {
            key._inputType = UIGamepadInputType::down;
            break;
        }
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: {
            key._inputType = UIGamepadInputType::right;
            break;
        }
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: {
            key._inputType = UIGamepadInputType::left;
            break;
        }
        case SDL_CONTROLLER_BUTTON_A: {
            key._inputType = UIGamepadInputType::buttonA;
            break;
        }
        case SDL_CONTROLLER_BUTTON_B: {
            key._inputType = UIGamepadInputType::buttonB;
            break;
        }
        case SDL_CONTROLLER_BUTTON_X: {
            key._inputType = UIGamepadInputType::buttonX;
            break;
        }
        case SDL_CONTROLLER_BUTTON_Y: {
            key._inputType = UIGamepadInputType::buttonY;
            break;
        }
        case SDL_CONTROLLER_BUTTON_START: {
            key._inputType = UIGamepadInputType::buttonStart;
            break;
        }
        case SDL_CONTROLLER_BUTTON_BACK: {
            key._inputType = UIGamepadInputType::buttonOptions;
            break;
        }
        case SDL_CONTROLLER_BUTTON_GUIDE: {
            key._inputType = UIGamepadInputType::buttonGuide;
            break;
        }
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: {
            key._inputType = UIGamepadInputType::leftShoulder;
            break;
        }
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: {
            key._inputType = UIGamepadInputType::rightShoulder;
            break;
        }
        case SDL_CONTROLLER_BUTTON_LEFTSTICK: {
            key._inputType = UIGamepadInputType::leftThumbstickButton;
            break;
        }
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK: {
            key._inputType = UIGamepadInputType::rightThumbstickButton;
            break;
        }
        default: break;
    }

    return key;
}

std::optional<UIGamepadKey> UIApplication::mapControllerAxisEventToUIGamepadKey(SDL_ControllerAxisEvent event) {
    UIGamepadKey key;
    key._value = float(event.value) / 32767.0f;

    switch (event.axis) {
        case SDL_CONTROLLER_AXIS_LEFTX: {
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
        case SDL_CONTROLLER_AXIS_LEFTY: {
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
        case SDL_CONTROLLER_AXIS_RIGHTX: {
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
        case SDL_CONTROLLER_AXIS_RIGHTY: {
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
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT: {
            key._inputType = UIGamepadInputType::leftTrigger;
            break;
        }
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: {
            key._inputType = UIGamepadInputType::rightTrigger;
            break;
        }
    }

    key._value = abs(key._value);
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
#else
        case UIGamepadInputType::buttonA:
            return UIPressType::select;
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

void UIApplication::handleSDLEvent(SDL_Event e) {
    auto screenSize = SkiaCtx::main()->getSize();
    switch (e.type) {
        case SDL_QUIT: {
            handleSDLQuit();
            return;
        }
        case SDL_FINGERDOWN: {
            auto renderSize = screenSize;
            auto fingerPoint = NXPoint(renderSize.width * e.tfinger.x, renderSize.height * e.tfinger.y);
//                printf("Touch id: %lld Begin, X:%f - Y:%f\n", e.tfinger.fingerId, fingerPoint.x, fingerPoint.y);
            auto touch = new_shared<UITouch>(e.tfinger.fingerId, fingerPoint, Timer());
            auto event = std::shared_ptr<UIEvent>(new UIEvent(touch));
            UIEvent::activeEvents.push_back(event);
            sendEvent(event);
            break;
        }
        case SDL_FINGERMOTION: {
            auto renderSize = screenSize;
            auto fingerPoint = NXPoint(renderSize.width * e.tfinger.x, renderSize.height * e.tfinger.y);
//                printf("Touch id: %lld Moved, X:%f - Y:%f\n", e.tfinger.fingerId, fingerPoint.x, fingerPoint.y);

            std::shared_ptr<UIEvent> event;
            std::shared_ptr<UITouch> touch;

            for (auto& levent: UIEvent::activeEvents) {
                for (auto& ltouch: levent->allTouches()) {
                    if (ltouch->touchId() == e.tfinger.fingerId) {
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

            // SDL adds timestamps on send which could be quite different to when the event actually occurred.
            // It's common to get two events with an unrealistically small time between them; don't send those.
            if ((newTimestamp - previousTimestamp) > (5 / 1000)) {
                sendEvent(event);
            }

            break;
        }
        case SDL_FINGERUP: {
//                printf("Touch id: %lld Ended\n", e.tfinger.fingerId);

            std::shared_ptr<UIEvent> event;
            std::shared_ptr<UITouch> touch;

            for (auto& levent: UIEvent::activeEvents) {
                for (auto& ltouch: levent->allTouches()) {
                    if (ltouch->touchId() == e.tfinger.fingerId) {
                        event = levent;
                        touch = ltouch;
                    }
                }
            }

            if (!event || !touch) return;

            touch->_timestamp = Timer();
            touch->_phase = UITouchPhase::ended;

            sendEvent(event);
            UIEvent::activeEvents.erase(std::remove(UIEvent::activeEvents.begin(), UIEvent::activeEvents.end(), event), UIEvent::activeEvents.end());

            break;
        }
#ifndef PLATFORM_IOS
        case SDL_MOUSEBUTTONDOWN: {
            // Simulate touch
            auto touchEvent = SDL_Event();
            touchEvent.type = SDL_FINGERDOWN;

            auto renderSize = screenSize;
            touchEvent.tfinger.x = float(e.button.x) / renderSize.width;
            touchEvent.tfinger.y = float(e.button.y) / renderSize.height;
            touchEvent.tfinger.fingerId = -1;

            handleSDLEvent(touchEvent);
            break;
        }
        case SDL_MOUSEMOTION: {
            // Simulate touch
            auto touchEvent = SDL_Event();
            touchEvent.type = SDL_FINGERMOTION;

            auto renderSize = screenSize;
            touchEvent.tfinger.x = float(e.button.x) / renderSize.width;
            touchEvent.tfinger.y = float(e.button.y) / renderSize.height;
            touchEvent.tfinger.fingerId = -1;

            handleSDLEvent(touchEvent);
            break;
        }
        case SDL_MOUSEBUTTONUP: {
            // Simulate touch
            auto touchEvent = SDL_Event();
            touchEvent.type = SDL_FINGERUP;

            auto renderSize = screenSize;
            touchEvent.tfinger.x = float(e.button.x) / renderSize.width;
            touchEvent.tfinger.y = float(e.button.y) / renderSize.height;
            touchEvent.tfinger.fingerId = -1;

            handleSDLEvent(touchEvent);
            break;
        }
#endif
        case SDL_CONTROLLERDEVICEADDED: {
            printf("Controller added\n");

            SDL_GameController* controller = SDL_GameControllerOpen(e.cdevice.which);
            if (controller)
            {
                SDL_JoystickID jid = SDL_JoystickGetDeviceInstanceID(e.cdevice.which);
                controllers.emplace( jid, controller );
            }

            break;
        }
        case SDL_CONTROLLERDEVICEREMOVED: {
            printf("Controller removed\n");
            auto controller = controllers[e.cdevice.which];

            SDL_GameControllerClose(controller);
            SDL_JoystickID jid = SDL_JoystickGetDeviceInstanceID(e.cdevice.which);
            controllers.erase(jid);
            break;
        }
        case SDL_CONTROLLERBUTTONDOWN: {
            printf("Controller button pressed\n");
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                handleSDLQuit();
            }

            auto press = new_shared<UIPress>();
            press->_phase = UIPressPhase::began;
            press->setForWindow(delegate->window);
            press->_gamepadKey = mapControllerButtonEventToUIGamepadKey(e.cbutton);
            press->_type = mapGamepadInputToUIPressType(press->_gamepadKey->inputType());

            auto event = std::shared_ptr<UIPressesEvent>(new UIPressesEvent(press));
            UIPressesEvent::activePressesEvents.push_back(event);
            sendEvent(event);

            break;
        }
        case SDL_CONTROLLERBUTTONUP: {
            printf("Controller button released\n");

            std::shared_ptr<UIPressesEvent> event;
            std::shared_ptr<UIPress> press;

            UIGamepadKey gamepadKey = mapControllerButtonEventToUIGamepadKey(e.cbutton);
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
        case SDL_CONTROLLERAXISMOTION: {
            printf("Controller axis moved\n");

            static std::map<UIGamepadInputType, std::shared_ptr<UIPress>> _pressesMap;

            auto gamepadKey = mapControllerAxisEventToUIGamepadKey(e.caxis);
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
        case SDL_JOYBUTTONDOWN: {
            printf("Joystick button pressed\n");
            break;
        }
        case SDL_JOYBUTTONUP: {
            printf("Joystick button released\n");
            break;
        }
        case SDL_KEYDOWN: {
            if (e.key.keysym.sym == SDLK_q) {
                handleSDLQuit();
            }

            auto press = new_shared<UIPress>();
            auto key = UIKey();
            key._keyCode = (UIKeyboardHIDUsage) e.key.keysym.scancode;
            key._modifierFlags = OptionSet<UIKeyModifierFlags>(e.key.keysym.mod);
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
                default: break;
            }

            auto event = std::shared_ptr<UIPressesEvent>(new UIPressesEvent(press));
            UIPressesEvent::activePressesEvents.push_back(event);
            sendEvent(event);

            break;
        }
        case SDL_KEYUP: {
            std::shared_ptr<UIPressesEvent> event;
            std::shared_ptr<UIPress> press;

            for (auto& levent: UIPressesEvent::activePressesEvents) {
                for (auto& lpress: levent->allPresses()) {
                    if (!lpress->_key.has_value()) continue;

                    if ((int) lpress->_key->_keyCode == (int) e.key.keysym.scancode) {
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
//                let keyModifier = SDL_Keymod(UInt32(e.key.keysym.mod))
//                if keyModifier.contains(KMOD_LSHIFT) || keyModifier.contains(KMOD_RSHIFT) {
//                    switch e.key.keysym.sym {
//                    case 43, 61: // +/*, +/= keys. TODO send key events via UIEvent
//                        break
//                    case 45: break // -/_ key
//                    case 118: // "V"
//                        keyWindow?.printViewHierarchy()
//                    default:
//                        print(e.key.keysym.sym)
//                    }
//                }
//
//                if keyModifier.contains(KMOD_LGUI) || keyModifier.contains(KMOD_RGUI) {
//                    if e.key.keysym.sym == 114 { // CMD-R
//                        UIScreen.main = nil
//                        UIScreen.main = UIScreen()
//                    }
//                }
//#endif
//
//                let scancode = e.key.keysym.scancode
//                if scancode == .androidHardwareBackButton || scancode == .escapeKey {
//                    keyWindow?.deepestPresentedView().handleHardwareBackButtonPress()
//                }
        }
        case SDL_APP_WILLENTERBACKGROUND: {
            UIApplication::onWillEnterBackground();
            break;
        }
        case SDL_APP_DIDENTERBACKGROUND: {
            UIApplication::onDidEnterBackground();
            break;
        }
        case SDL_APP_WILLENTERFOREGROUND: {
            UIApplication::onWillEnterForeground();
            break;
        }
        case SDL_APP_DIDENTERFOREGROUND: {
            UIApplication::onDidEnterForeground();
            break;
        }
        case SDL_WINDOWEVENT: {
            switch (e.window.event)
            {
                case SDL_WINDOWEVENT_RESIZED:
//                    UIRenderer::main()->refreshScreenResolution(e.window.data1, e.window.data2);
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

void UIApplication::handleSDLQuit() {
    if (delegate) delegate->applicationWillTerminate(this);

    UIEvent::activeEvents.clear();
    UIApplication::delegate->window = nullptr;
    UIApplication::shared = nullptr;
    // UIRenderer::_main = nullptr;

    SDL_Quit();
    exit(0);
}

 void UIApplication::sendEvent(const std::shared_ptr<UIEvent>& event) const {
     for (auto& touch: event->allTouches()) {
         touch->_window = keyWindow;
     }

     if (!keyWindow.expired())
         keyWindow.lock()->sendEvent(event);
 }

void UIApplication::onWillEnterForeground() {
    UIApplication::shared->delegate->applicationDidEnterBackground(UIApplication::shared.get());
//    UIApplication.post(willEnterForegroundNotification)
}

void UIApplication::onDidEnterForeground() {
    UIApplication::shared->delegate->applicationDidBecomeActive(UIApplication::shared.get());
//    UIApplication.post(didBecomeActiveNotification)
}

void UIApplication::onWillEnterBackground() {
    UIApplication::shared->delegate->applicationWillResignActive(UIApplication::shared.get());
//    UIApplication.post(willResignActiveNotification)
}

void UIApplication::onDidEnterBackground() {
    UIApplication::shared->delegate->applicationDidEnterBackground(UIApplication::shared.get());
//    UIApplication.post(didEnterBackgroundNotification)
}