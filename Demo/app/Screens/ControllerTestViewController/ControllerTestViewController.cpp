#include <Screens/ControllerTestViewController/ControllerTestViewController.hpp>

#include <CADisplayLink.h>
#include <DispatchQueue.h>
#include <NXControllerIconResolver.h>
#include <NXNavigationController.h>
#include <NXResponderAction.h>
#include <UIButton.h>
#include <UIImageView.h>
#include <UILabel.h>
#include <UIPress.h>
#include <UIPressesEvent.h>
#include <UIView.h>

#include <SDL3/SDL.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace NXKit;
using namespace NXKit::yoga::literals;

namespace {

using Clock = std::chrono::steady_clock;

constexpr NXFloat contentMargin = 28;
constexpr NXFloat iconRowHeight = 76;
constexpr NXFloat eventIconSize = 52;
constexpr NXFloat eventIconSpacing = 12;
constexpr NXFloat controlSpacing = 22;
constexpr NXFloat gaugeWidth = 62;
constexpr NXFloat stickIndicatorSize = 180;
constexpr NXFloat exitButtonWidth = 150;
constexpr NXFloat exitButtonHeight = 52;
constexpr double iconVisibleDuration = 3.0;
constexpr double iconFadeDuration = 2.0;
constexpr double exitHoldDuration = 3.0;

std::shared_ptr<NXNavigationController> enclosingNavigationController(
    const std::shared_ptr<UIViewController>& controller
) {
    for (auto current = controller; current; current = current->parent().lock()) {
        if (const auto navigationController =
                std::dynamic_pointer_cast<NXNavigationController>(current)) {
            return navigationController;
        }
    }
    return nullptr;
}

UIEdgeInsets resolvedNavigationSafeArea(
    const std::shared_ptr<UIViewController>& controller,
    const std::shared_ptr<UIView>& controllerView
) {
    const auto viewInsets = controllerView
        ? controllerView->safeAreaInsets()
        : UIEdgeInsets::zero;
    const auto navigationController = enclosingNavigationController(controller);
    if (!navigationController || !navigationController->viewIsLoaded()) {
        return viewInsets;
    }

    const auto navigationView = navigationController->view();
    const auto chromeInsets = navigationController->additionalSafeAreaInsets();
    auto systemInsets = UIEdgeInsets::zero;
    if (const auto navigationSuperview = navigationView->superview().lock()) {
        systemInsets = navigationSuperview->safeAreaInsets();
    }

    // The child view's cached safe area can be either one layout behind or
    // temporarily over-inset while the navigation transition reparents it.
    // The enclosing navigation controller is authoritative for this screen:
    // its superview supplies device insets and its additional insets describe
    // the translucent header and footer exactly.
    return systemInsets + chromeInsets;
}

std::optional<NXActionButton> actionButtonForGamepadInput(
    UIGamepadInputType input
) {
    switch (input) {
#if defined(PLATFORM_SWITCH)
        case UIGamepadInputType::buttonA: return NXActionButton::b;
        case UIGamepadInputType::buttonB: return NXActionButton::a;
        case UIGamepadInputType::buttonX: return NXActionButton::y;
        case UIGamepadInputType::buttonY: return NXActionButton::x;
#else
        case UIGamepadInputType::buttonA: return NXActionButton::a;
        case UIGamepadInputType::buttonB: return NXActionButton::b;
        case UIGamepadInputType::buttonX: return NXActionButton::x;
        case UIGamepadInputType::buttonY: return NXActionButton::y;
#endif
        case UIGamepadInputType::buttonStart: return NXActionButton::plus;
        case UIGamepadInputType::buttonOptions: return NXActionButton::minus;
        case UIGamepadInputType::buttonGuide: return NXActionButton::home;
        case UIGamepadInputType::leftThumbstickButton:
            return NXActionButton::leftThumbstick;
        case UIGamepadInputType::rightThumbstickButton:
            return NXActionButton::rightThumbstick;
        case UIGamepadInputType::leftShoulder:
            return NXActionButton::leftShoulder;
        case UIGamepadInputType::rightShoulder:
            return NXActionButton::rightShoulder;
        case UIGamepadInputType::leftTrigger:
            return NXActionButton::leftTrigger;
        case UIGamepadInputType::rightTrigger:
            return NXActionButton::rightTrigger;
        case UIGamepadInputType::up: return NXActionButton::dpadUp;
        case UIGamepadInputType::down: return NXActionButton::dpadDown;
        case UIGamepadInputType::left: return NXActionButton::dpadLeft;
        case UIGamepadInputType::right: return NXActionButton::dpadRight;
        case UIGamepadInputType::misc1: return NXActionButton::misc1;
        case UIGamepadInputType::rightPaddle1:
            return NXActionButton::rightPaddle1;
        case UIGamepadInputType::leftPaddle1:
            return NXActionButton::leftPaddle1;
        case UIGamepadInputType::rightPaddle2:
            return NXActionButton::rightPaddle2;
        case UIGamepadInputType::leftPaddle2:
            return NXActionButton::leftPaddle2;
        case UIGamepadInputType::touchpad: return NXActionButton::touchpad;
        case UIGamepadInputType::misc2: return NXActionButton::misc2;
        case UIGamepadInputType::misc3: return NXActionButton::misc3;
        case UIGamepadInputType::misc4: return NXActionButton::misc4;
        case UIGamepadInputType::misc5: return NXActionButton::misc5;
        case UIGamepadInputType::misc6: return NXActionButton::misc6;
        case UIGamepadInputType::leftThumbstickAxisLeft:
        case UIGamepadInputType::leftThumbstickAxisRight:
        case UIGamepadInputType::leftThumbstickAxisUp:
        case UIGamepadInputType::leftThumbstickAxisDown:
        case UIGamepadInputType::rightThumbstickAxisLeft:
        case UIGamepadInputType::rightThumbstickAxisRight:
        case UIGamepadInputType::rightThumbstickAxisUp:
        case UIGamepadInputType::rightThumbstickAxisDown:
        case UIGamepadInputType::unknown:
        default: return std::nullopt;
    }
}

Uint8 sdlBackButton() {
#if defined(PLATFORM_SWITCH)
    return SDL_GAMEPAD_BUTTON_SOUTH;
#else
    return SDL_GAMEPAD_BUTTON_EAST;
#endif
}

float normalizedStickValue(Sint16 value) {
    const auto normalized = value < 0
        ? static_cast<float>(value) / 32768.0f
        : static_cast<float>(value) / 32767.0f;
    return std::clamp(normalized, -1.0f, 1.0f);
}

float normalizedTriggerValue(Sint16 value) {
    return std::clamp(static_cast<float>(value) / 32767.0f, 0.0f, 1.0f);
}

class ControllerCaptureView final : public UIView {
public:
    void setButtonPressHandler(
        std::function<void(NXActionButton)> buttonPressHandler
    ) {
        _buttonPressHandler = std::move(buttonPressHandler);
    }

    bool canBecomeFocused() override { return true; }

    void pressesBegan(
        std::set<std::shared_ptr<UIPress>> presses,
        std::shared_ptr<UIPressesEvent> event
    ) override {
        for (const auto& press : presses) {
            if (!press || press->isRepeat()
                || !_activePresses.insert(press.get()).second) {
                continue;
            }
            const auto gamepadKey = press->gamepadKey();
            if (!gamepadKey) {
                continue;
            }
            if (const auto button = actionButtonForGamepadInput(
                    gamepadKey->inputType()
                ); button && _buttonPressHandler) {
                _buttonPressHandler(*button);
            }
        }
        UIView::pressesBegan(std::move(presses), std::move(event));
    }

    void pressesEnded(
        std::set<std::shared_ptr<UIPress>> presses,
        std::shared_ptr<UIPressesEvent> event
    ) override {
        removeActivePresses(presses);
        UIView::pressesEnded(std::move(presses), std::move(event));
    }

    void pressesCancelled(
        std::set<std::shared_ptr<UIPress>> presses,
        std::shared_ptr<UIPressesEvent> event
    ) override {
        removeActivePresses(presses);
        UIView::pressesCancelled(std::move(presses), std::move(event));
    }

private:
    void removeActivePresses(
        const std::set<std::shared_ptr<UIPress>>& presses
    ) {
        for (const auto& press : presses) {
            _activePresses.erase(press.get());
        }
    }

    std::function<void(NXActionButton)> _buttonPressHandler;
    std::set<const UIPress*> _activePresses;
};

class TouchOnlyButton final : public UIButton {
public:
    explicit TouchOnlyButton(UIButtonStyle style) : UIButton(style) {}

    bool canBecomeFocused() override { return false; }
};

class StickIndicatorView final : public UIView {
public:
    StickIndicatorView() {
        setUserInteractionEnabled(false);
        setBackgroundColor(UIColor::secondarySystemFill);
        layer()->setBorderColor(UIColor::separator);
        layer()->setBorderWidth(2);

        _dot = new_shared<UIView>();
        _dot->setUserInteractionEnabled(false);
        _dot->setBackgroundColor(UIColor::tint);
        _dot->layer()->setShadowColor(UIColor::black);
        _dot->layer()->setShadowOpacity(0.25f);
        _dot->layer()->setShadowOffset(NXPoint(0, 3));
        _dot->layer()->setShadowRadius(5);
        addSubview(_dot);
    }

    void setPosition(NXPoint position) {
        position.x = std::clamp(position.x, -1.0f, 1.0f);
        position.y = std::clamp(position.y, -1.0f, 1.0f);
        const auto magnitude = std::sqrt(
            position.x * position.x + position.y * position.y
        );
        if (magnitude > 1.0f) {
            position.x /= magnitude;
            position.y /= magnitude;
        }
        if (_position == position) {
            return;
        }
        _position = position;
        layoutDot();
    }

    void layoutSubviews() override {
        UIView::layoutSubviews();
        const auto diameter = std::min(bounds().width(), bounds().height());
        layer()->setCornerRadius(diameter * 0.5f);
        layoutDot();
    }

private:
    void layoutDot() {
        constexpr NXFloat dotSize = 20;
        constexpr NXFloat inset = 12;
        const auto boundsSize = bounds().size;
        const auto travel = std::max<NXFloat>(
            0,
            std::min(boundsSize.width, boundsSize.height) * 0.5f
                - dotSize * 0.5f
                - inset
        );
        _dot->setFrame(NXRect(
            boundsSize.width * 0.5f - dotSize * 0.5f + _position.x * travel,
            boundsSize.height * 0.5f - dotSize * 0.5f + _position.y * travel,
            dotSize,
            dotSize
        ));
        _dot->layer()->setCornerRadius(dotSize * 0.5f);
    }

    std::shared_ptr<UIView> _dot;
    NXPoint _position;
};

class TriggerGaugeView final : public UIView {
public:
    TriggerGaugeView() {
        setUserInteractionEnabled(false);

        _iconView = new_shared<UIImageView>();
        _iconView->setContentMode(UIViewContentMode::scaleAspectFit);
        addSubview(_iconView);

        _track = new_shared<UIView>();
        _track->setUserInteractionEnabled(false);
        _track->setBackgroundColor(UIColor::secondarySystemFill);
        _track->setClipsToBounds(true);
        _track->layer()->setBorderColor(UIColor::separator);
        _track->layer()->setBorderWidth(1);
        addSubview(_track);

        _fill = new_shared<UIView>();
        _fill->setUserInteractionEnabled(false);
        _fill->setBackgroundColor(UIColor::tint);
        _track->addSubview(_fill);

        _valueLabel = new_shared<UILabel>();
        _valueLabel->setUserInteractionEnabled(false);
        _valueLabel->setFontSize(13);
        _valueLabel->setTextAlignment(NSTextAlignment::center);
        _valueLabel->setTextColor(UIColor::secondaryLabel);
        _valueLabel->setText("0%");
        addSubview(_valueLabel);
    }

    void setImage(const std::shared_ptr<UIImage>& image) {
        _iconView->setImage(image);
    }

    void setValue(float value) {
        value = std::clamp(value, 0.0f, 1.0f);
        if (std::abs(value - _value) < 0.0001f) {
            return;
        }
        _value = value;
        _valueLabel->setText(
            std::to_string(static_cast<int>(std::round(_value * 100))) + "%"
        );
        layoutFill();
    }

    void layoutSubviews() override {
        UIView::layoutSubviews();
        const auto size = bounds().size;
        constexpr NXFloat iconSize = 34;
        constexpr NXFloat labelHeight = 20;
        constexpr NXFloat verticalGap = 8;
        constexpr NXFloat trackWidth = 24;
        const auto trackTop = iconSize + verticalGap;
        const auto trackHeight = std::max<NXFloat>(
            0,
            size.height - trackTop - labelHeight - verticalGap
        );

        _iconView->setFrame(NXRect(
            (size.width - iconSize) * 0.5f,
            0,
            iconSize,
            iconSize
        ));
        _track->setFrame(NXRect(
            (size.width - trackWidth) * 0.5f,
            trackTop,
            trackWidth,
            trackHeight
        ));
        _track->layer()->setCornerRadius(trackWidth * 0.5f);
        _valueLabel->setFrame(NXRect(
            0,
            size.height - labelHeight,
            size.width,
            labelHeight
        ));
        layoutFill();
    }

private:
    void layoutFill() {
        const auto trackBounds = _track->bounds();
        const auto fillHeight = trackBounds.height() * _value;
        _fill->setFrame(NXRect(
            0,
            trackBounds.height() - fillHeight,
            trackBounds.width(),
            fillHeight
        ));
    }

    std::shared_ptr<UIImageView> _iconView;
    std::shared_ptr<UIView> _track;
    std::shared_ptr<UIView> _fill;
    std::shared_ptr<UILabel> _valueLabel;
    float _value = 0;
};

class ControllerInputTestViewController final : public UIViewController {
public:
    ControllerInputTestViewController() {
        setTitle("Controller input");
    }

    ~ControllerInputTestViewController() override {
        stopObservingController();
    }

    void loadView() override {
        auto rootView = new_shared<ControllerCaptureView>();
        rootView->setBackgroundColor(UIColor::systemBackground);
        rootView->setClipsToBounds(true);
        const auto weakSelf = weak_from_base<ControllerInputTestViewController>();
        rootView->setButtonPressHandler([weakSelf](NXActionButton button) {
            if (const auto self = weakSelf.lock()) {
                self->recordButtonPress(button);
            }
        });

        _contentContainer = new_shared<UIView>();
        _contentContainer->setUserInteractionEnabled(true);
        _contentContainer->configureLayout([](
            const std::shared_ptr<YGLayout>& layout
        ) {
            layout->setPositionType(YGPositionTypeAbsolute);
            layout->setLeft(0_pt);
            layout->setTop(0_pt);
            layout->setWidth(100_percent);
            layout->setHeight(100_percent);
            layout->setFlexDirection(YGFlexDirectionColumn);
            layout->setAllGap(20);
        });
        rootView->addSubview(_contentContainer);

        _eventRow = new_shared<UIView>();
        _eventRow->setUserInteractionEnabled(false);
        _eventRow->setBackgroundColor(UIColor::tertiarySystemFill);
        _eventRow->setClipsToBounds(true);
        _eventRow->layer()->setCornerRadius(18);
        _eventRow->layer()->setBorderColor(UIColor::separator);
        _eventRow->layer()->setBorderWidth(1);
        _eventRow->configureLayout([](
            const std::shared_ptr<YGLayout>& layout
        ) {
            layout->setWidth(100_percent);
            layout->setHeight(YGValue {
                static_cast<float>(iconRowHeight),
                YGUnitPoint,
            });
            layout->setFlexShrink(0);
            layout->setAlignItems(YGAlignCenter);
            layout->setJustifyContent(YGJustifyCenter);
        });
        _contentContainer->addSubview(_eventRow);

        _eventIconStrip = new_shared<UIView>();
        _eventIconStrip->setUserInteractionEnabled(false);
        _eventIconStrip->configureLayout([](
            const std::shared_ptr<YGLayout>& layout
        ) {
            layout->setPositionType(YGPositionTypeAbsolute);
            layout->setLeft(0_pt);
            layout->setTop(0_pt);
            layout->setWidth(100_percent);
            layout->setHeight(100_percent);
            layout->setFlexDirection(YGFlexDirectionRow);
            layout->setAlignItems(YGAlignCenter);
            layout->setJustifyContent(YGJustifyFlexEnd);
            layout->setAllGap(static_cast<float>(eventIconSpacing));
            layout->setPaddingHorizontal(YGValue {
                static_cast<float>(eventIconSpacing),
                YGUnitPoint,
            });
        });
        _eventRow->addSubview(_eventIconStrip);

        _emptyRowLabel = new_shared<UILabel>();
        _emptyRowLabel->setUserInteractionEnabled(false);
        _emptyRowLabel->setText("Press any controller button");
        _emptyRowLabel->setTextAlignment(NSTextAlignment::center);
        _emptyRowLabel->setTextColor(UIColor::secondaryLabel);
        _emptyRowLabel->setFontSize(18);
        _emptyRowLabel->setAutolayoutEnabled(true);
        _eventRow->addSubview(_emptyRowLabel);

        auto controlsRow = new_shared<UIView>();
        controlsRow->setUserInteractionEnabled(false);
        controlsRow->configureLayout([](
            const std::shared_ptr<YGLayout>& layout
        ) {
            layout->setWidth(100_percent);
            layout->setFlexDirection(YGFlexDirectionRow);
            layout->setAlignItems(YGAlignCenter);
            layout->setJustifyContent(YGJustifyCenter);
            layout->setAllGap(static_cast<float>(controlSpacing));
            layout->setFlexGrow(1);
            layout->setFlexShrink(1);
            layout->setFlexBasis(0_pt);
        });
        _contentContainer->addSubview(controlsRow);

        auto leftStickGroup = makeStickGroup();
        controlsRow->addSubview(leftStickGroup);

        _leftStick = new_shared<StickIndicatorView>();
        configureStickLayout(_leftStick);
        leftStickGroup->addSubview(_leftStick);

        _leftStickLabel = new_shared<UILabel>();
        configureStickLabel(_leftStickLabel, "Left stick");
        leftStickGroup->addSubview(_leftStickLabel);

        _leftTrigger = new_shared<TriggerGaugeView>();
        configureGaugeLayout(_leftTrigger);
        controlsRow->addSubview(_leftTrigger);

        _rightTrigger = new_shared<TriggerGaugeView>();
        configureGaugeLayout(_rightTrigger);
        controlsRow->addSubview(_rightTrigger);

        auto rightStickGroup = makeStickGroup();
        controlsRow->addSubview(rightStickGroup);

        _rightStick = new_shared<StickIndicatorView>();
        configureStickLayout(_rightStick);
        rightStickGroup->addSubview(_rightStick);

        _rightStickLabel = new_shared<UILabel>();
        configureStickLabel(_rightStickLabel, "Right stick");
        rightStickGroup->addSubview(_rightStickLabel);

        auto footerRow = new_shared<UIView>();
        footerRow->setUserInteractionEnabled(true);
        footerRow->configureLayout([](
            const std::shared_ptr<YGLayout>& layout
        ) {
            layout->setWidth(100_percent);
            layout->setHeight(YGValue {
                static_cast<float>(exitButtonHeight),
                YGUnitPoint,
            });
            layout->setFlexDirection(YGFlexDirectionRow);
            layout->setAlignItems(YGAlignCenter);
            layout->setAllGap(16);
            layout->setFlexShrink(0);
        });
        _contentContainer->addSubview(footerRow);

        auto holdGroup = new_shared<UIView>();
        holdGroup->setUserInteractionEnabled(false);
        holdGroup->configureLayout([](
            const std::shared_ptr<YGLayout>& layout
        ) {
            layout->setFlexDirection(YGFlexDirectionRow);
            layout->setAlignItems(YGAlignCenter);
            layout->setAllGap(10);
            layout->setFlexGrow(1);
            layout->setFlexShrink(1);
        });
        footerRow->addSubview(holdGroup);

        _holdIconView = new_shared<UIImageView>();
        _holdIconView->setUserInteractionEnabled(false);
        _holdIconView->setContentMode(UIViewContentMode::scaleAspectFit);
        _holdIconView->configureLayout([](
            const std::shared_ptr<YGLayout>& layout
        ) {
            layout->setWidth(32_pt);
            layout->setHeight(32_pt);
            layout->setFlexShrink(0);
        });
        holdGroup->addSubview(_holdIconView);

        _holdStatusLabel = new_shared<UILabel>();
        _holdStatusLabel->setUserInteractionEnabled(false);
        _holdStatusLabel->setText("Hold for 3 seconds to go back");
        _holdStatusLabel->setTextColor(UIColor::secondaryLabel);
        _holdStatusLabel->setFontSize(16);
        _holdStatusLabel->configureLayout([](
            const std::shared_ptr<YGLayout>& layout
        ) {
            layout->setFlexShrink(1);
        });
        holdGroup->addSubview(_holdStatusLabel);

        _exitButton = new_shared<TouchOnlyButton>(UIButtonStyle::tinted);
        _exitButton->setText("Exit");
        _exitButton->layer()->setCornerRadius(10);
        _exitButton->configureLayout([](
            const std::shared_ptr<YGLayout>& layout
        ) {
            layout->setWidth(YGValue {
                static_cast<float>(exitButtonWidth),
                YGUnitPoint,
            });
            layout->setHeight(YGValue {
                static_cast<float>(exitButtonHeight),
                YGUnitPoint,
            });
            layout->setFlexShrink(0);
        });
        _exitButton->primaryAction = UIAction("Exit", [weakSelf]() {
            if (const auto self = weakSelf.lock()) {
                self->exitScreen();
            }
        });
        footerRow->addSubview(_exitButton);

        NXResponderAction {
            .button = NXActionButton::b,
            .isEnabled = false,
            .action = UIAction("Hold 3s to exit", []() {}),
            .identifier = "Demo.controllerInput.holdToExit",
            .priority = 1000,
        }.registerOn(rootView);

        setView(rootView);
        updateSafeAreaLayout();
        refreshControllerSpecificIcons();
    }

    void viewWillAppear(bool animated) override {
        UIViewController::viewWillAppear(animated);
        updateSafeAreaLayout();
        startObservingController();
    }

    void viewDidAppear(bool animated) override {
        UIViewController::viewDidAppear(animated);
    }

    void viewWillDisappear(bool animated) override {
        stopObservingController();
        UIViewController::viewWillDisappear(animated);
    }

    void viewSafeAreaInsetsDidChange() override {
        UIViewController::viewSafeAreaInsetsDidChange();
        updateSafeAreaLayout();
    }

    void viewDidLayoutSubviews() override {
        UIViewController::viewDidLayoutSubviews();
    }

private:
    void updateSafeAreaLayout() {
        if (!viewIsLoaded() || !_contentContainer) {
            return;
        }
        const auto safeArea = resolvedNavigationSafeArea(
            shared_from_base<ControllerInputTestViewController>(),
            view()
        );
        _contentContainer->configureLayout([
            safeArea
        ](const std::shared_ptr<YGLayout>& layout) {
            layout->setPaddingTop(YGValue {
                static_cast<float>(safeArea.top + contentMargin),
                YGUnitPoint,
            });
            layout->setPaddingLeft(YGValue {
                static_cast<float>(safeArea.left + contentMargin),
                YGUnitPoint,
            });
            layout->setPaddingBottom(YGValue {
                static_cast<float>(safeArea.bottom + contentMargin),
                YGUnitPoint,
            });
            layout->setPaddingRight(YGValue {
                static_cast<float>(safeArea.right + contentMargin),
                YGUnitPoint,
            });
        });
    }

    struct EventIcon {
        std::shared_ptr<UIImageView> imageView;
        Clock::time_point appearedAt;
    };

    static void configureStickLabel(
        const std::shared_ptr<UILabel>& label,
        const std::string& text
    ) {
        label->setUserInteractionEnabled(false);
        label->setText(text);
        label->setTextAlignment(NSTextAlignment::center);
        label->setTextColor(UIColor::secondaryLabel);
        label->setFontSize(15);
        label->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setWidth(100_percent);
            layout->setHeight(24_pt);
            layout->setFlexShrink(0);
        });
    }

    static std::shared_ptr<UIView> makeStickGroup() {
        auto group = new_shared<UIView>();
        group->setUserInteractionEnabled(false);
        group->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setWidth(YGValue {
                static_cast<float>(stickIndicatorSize),
                YGUnitPoint,
            });
            layout->setFlexDirection(YGFlexDirectionColumn);
            layout->setAlignItems(YGAlignCenter);
            layout->setAllGap(8);
            layout->setFlexShrink(0);
        });
        return group;
    }

    static void configureStickLayout(
        const std::shared_ptr<StickIndicatorView>& stick
    ) {
        stick->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setWidth(YGValue {
                static_cast<float>(stickIndicatorSize),
                YGUnitPoint,
            });
            layout->setHeight(YGValue {
                static_cast<float>(stickIndicatorSize),
                YGUnitPoint,
            });
            layout->setFlexShrink(0);
        });
    }

    static void configureGaugeLayout(
        const std::shared_ptr<TriggerGaugeView>& gauge
    ) {
        gauge->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setWidth(YGValue {
                static_cast<float>(gaugeWidth),
                YGUnitPoint,
            });
            layout->setHeight(YGValue {
                static_cast<float>(stickIndicatorSize),
                YGUnitPoint,
            });
            layout->setFlexShrink(0);
        });
    }

    static bool SDLCALL handleControllerEvent(void* userdata, SDL_Event* event) {
        auto* self = static_cast<ControllerInputTestViewController*>(userdata);
        if (self && event) {
            self->recordControllerEvent(*event);
        }
        return true;
    }

    void startObservingController() {
        if (_displayLink) {
            return;
        }

        {
            std::lock_guard lock(_inputMutex);
            _pendingButtons.clear();
            _buttonsDown.clear();
            _backButtonHolds.clear();
            _activeController = 0;
            _leftStickPosition = NXPoint::zero;
            _rightStickPosition = NXPoint::zero;
            _leftTriggerValue = 0;
            _rightTriggerValue = 0;
        }
        _exitRequested = false;

        const auto weakSelf = weak_from_base<ControllerInputTestViewController>();
        _displayLink = std::make_unique<CADisplayLink>([weakSelf]() {
            if (const auto self = weakSelf.lock()) {
                self->updateControllerDisplay();
            }
        });

        if (SDL_WasInit(SDL_INIT_EVENTS) == 0) {
            return;
        }
        _watchInstalled = SDL_AddEventWatch(handleControllerEvent, this);
        if (!_watchInstalled) {
            SDL_LogError(
                SDL_LOG_CATEGORY_INPUT,
                "Could not register controller test event watch: %s",
                SDL_GetError()
            );
        }
    }

    void stopObservingController() {
        _displayLink.reset();
        if (_watchInstalled) {
            SDL_RemoveEventWatch(handleControllerEvent, this);
            _watchInstalled = false;
        }
        std::lock_guard lock(_inputMutex);
        _buttonsDown.clear();
        _backButtonHolds.clear();
    }

    void recordButtonPress(NXActionButton button) {
        std::lock_guard lock(_inputMutex);
        _pendingButtons.push_back(button);
    }

    void selectActiveControllerLocked(SDL_JoystickID identifier) {
        if (_activeController == identifier) {
            return;
        }
        _activeController = identifier;
        _leftStickPosition = NXPoint::zero;
        _rightStickPosition = NXPoint::zero;
        _leftTriggerValue = 0;
        _rightTriggerValue = 0;
    }

    void recordControllerEvent(const SDL_Event& event) {
        std::lock_guard lock(_inputMutex);

        switch (event.type) {
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
                selectActiveControllerLocked(event.gbutton.which);
                const auto key = std::make_pair(
                    event.gbutton.which,
                    static_cast<int>(event.gbutton.button)
                );
                if (!_buttonsDown.insert(key).second) {
                    break;
                }
                if (event.gbutton.button == sdlBackButton()) {
                    _backButtonHolds[event.gbutton.which] = Clock::now();
                }
                break;
            }
            case SDL_EVENT_GAMEPAD_BUTTON_UP: {
                const auto key = std::make_pair(
                    event.gbutton.which,
                    static_cast<int>(event.gbutton.button)
                );
                _buttonsDown.erase(key);
                if (event.gbutton.button == sdlBackButton()) {
                    _backButtonHolds.erase(event.gbutton.which);
                }
                break;
            }
            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
                recordAxisEventLocked(event.gaxis);
                break;
            case SDL_EVENT_GAMEPAD_REMOVED:
                eraseControllerStateLocked(event.gdevice.which);
                break;
            case SDL_EVENT_DID_ENTER_BACKGROUND:
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                _buttonsDown.clear();
                _backButtonHolds.clear();
                break;
            default:
                break;
        }
    }

    void recordAxisEventLocked(const SDL_GamepadAxisEvent& event) {
        const auto meaningful = std::abs(static_cast<int>(event.value)) > 2500;
        if (_activeController != event.which && !meaningful) {
            return;
        }
        if (meaningful) {
            selectActiveControllerLocked(event.which);
        }
        if (_activeController != event.which) {
            return;
        }

        switch (event.axis) {
            case SDL_GAMEPAD_AXIS_LEFTX:
                _leftStickPosition.x = normalizedStickValue(event.value);
                break;
            case SDL_GAMEPAD_AXIS_LEFTY:
                _leftStickPosition.y = normalizedStickValue(event.value);
                break;
            case SDL_GAMEPAD_AXIS_RIGHTX:
                _rightStickPosition.x = normalizedStickValue(event.value);
                break;
            case SDL_GAMEPAD_AXIS_RIGHTY:
                _rightStickPosition.y = normalizedStickValue(event.value);
                break;
            case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
                _leftTriggerValue = normalizedTriggerValue(event.value);
                break;
            case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
                _rightTriggerValue = normalizedTriggerValue(event.value);
                break;
            default:
                break;
        }
    }

    void eraseControllerStateLocked(SDL_JoystickID identifier) {
        std::erase_if(_buttonsDown, [identifier](const auto& entry) {
            return entry.first == identifier;
        });
        _backButtonHolds.erase(identifier);
        if (_activeController == identifier) {
            _activeController = 0;
            _leftStickPosition = NXPoint::zero;
            _rightStickPosition = NXPoint::zero;
            _leftTriggerValue = 0;
            _rightTriggerValue = 0;
        }
    }

    void updateControllerDisplay() {
        std::vector<NXActionButton> buttons;
        NXPoint leftStick;
        NXPoint rightStick;
        float leftTrigger = 0;
        float rightTrigger = 0;
        double longestBackHold = 0;
        const auto now = Clock::now();

        {
            std::lock_guard lock(_inputMutex);
            buttons.swap(_pendingButtons);
            leftStick = _leftStickPosition;
            rightStick = _rightStickPosition;
            leftTrigger = _leftTriggerValue;
            rightTrigger = _rightTriggerValue;
            for (const auto& [identifier, beganAt] : _backButtonHolds) {
                (void)identifier;
                longestBackHold = std::max(
                    longestBackHold,
                    std::chrono::duration<double>(now - beganAt).count()
                );
            }
        }

        refreshControllerSpecificIcons();
        for (const auto button : buttons) {
            appendEventIcon(button, now);
        }

        _leftStick->setPosition(leftStick);
        _rightStick->setPosition(rightStick);
        _leftTrigger->setValue(leftTrigger);
        _rightTrigger->setValue(rightTrigger);
        updateEventIconLifetimes(now);
        updateHoldStatus(longestBackHold);

        if (!_exitRequested && longestBackHold >= exitHoldDuration) {
            exitScreen();
        }
    }

    void refreshControllerSpecificIcons() {
        const auto type = NXControllerIconResolver::shared().currentControllerType();
        if (type == _renderedControllerType) {
            return;
        }
        _renderedControllerType = type;
        _leftTrigger->setImage(
            NXControllerIconResolver::shared().iconForButton(
                NXActionButton::leftTrigger,
                type,
                34
            )
        );
        _rightTrigger->setImage(
            NXControllerIconResolver::shared().iconForButton(
                NXActionButton::rightTrigger,
                type,
                34
            )
        );
        _holdIconView->setImage(
            NXControllerIconResolver::shared().iconForButton(
                NXActionButton::b,
                type,
                32
            )
        );
    }

    void appendEventIcon(NXActionButton button, Clock::time_point now) {
        auto iconView = new_shared<UIImageView>(
            NXControllerIconResolver::shared().iconForButton(
                button,
                _renderedControllerType.value_or(
                    NXControllerType::automatic
                ),
                eventIconSize
            )
        );
        iconView->setContentMode(UIViewContentMode::scaleAspectFit);
        iconView->setUserInteractionEnabled(false);
        iconView->setTintColor(UIColor::label);
        iconView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setWidth(YGValue {
                static_cast<float>(eventIconSize),
                YGUnitPoint,
            });
            layout->setHeight(YGValue {
                static_cast<float>(eventIconSize),
                YGUnitPoint,
            });
            layout->setFlexShrink(0);
        });
        _eventIconStrip->addSubview(iconView);
        _eventIcons.push_back(EventIcon {
            .imageView = std::move(iconView),
            .appearedAt = now,
        });
        _emptyRowLabel->setAlpha(0);
    }

    void updateEventIconLifetimes(Clock::time_point now) {
        std::erase_if(_eventIcons, [&](const EventIcon& icon) {
            const auto age = std::chrono::duration<double>(
                now - icon.appearedAt
            ).count();
            if (age >= iconVisibleDuration + iconFadeDuration) {
                icon.imageView->removeFromSuperview();
                return true;
            }
            const auto alpha = age <= iconVisibleDuration
                ? 1.0
                : 1.0 - (age - iconVisibleDuration) / iconFadeDuration;
            icon.imageView->setAlpha(static_cast<NXFloat>(alpha));
            return false;
        });
        _emptyRowLabel->setAlpha(_eventIcons.empty() ? 1 : 0);
    }

    void updateHoldStatus(double heldDuration) {
        if (heldDuration <= 0) {
            _holdStatusLabel->setText("Hold for 3 seconds to go back");
            return;
        }
        const auto remaining = std::max(0.0, exitHoldDuration - heldDuration);
        const auto tenths = static_cast<int>(std::ceil(remaining * 10));
        _holdStatusLabel->setText(
            "Keep holding: "
                + std::to_string(tenths / 10)
                + "."
                + std::to_string(tenths % 10)
                + "s"
        );
    }

    void exitScreen() {
        if (_exitRequested) {
            return;
        }
        _exitRequested = true;
        const auto weakSelf = weak_from_base<ControllerInputTestViewController>();
        DispatchQueue::main()->async([weakSelf]() {
            const auto self = weakSelf.lock();
            if (!self) {
                return;
            }
            if (const auto navigationController = enclosingNavigationController(
                    self
                )) {
                navigationController->popViewController(true);
            }
        });
    }

    std::shared_ptr<UIView> _contentContainer;
    std::shared_ptr<UIView> _eventRow;
    std::shared_ptr<UIView> _eventIconStrip;
    std::shared_ptr<UILabel> _emptyRowLabel;
    std::shared_ptr<StickIndicatorView> _leftStick;
    std::shared_ptr<StickIndicatorView> _rightStick;
    std::shared_ptr<TriggerGaugeView> _leftTrigger;
    std::shared_ptr<TriggerGaugeView> _rightTrigger;
    std::shared_ptr<UILabel> _leftStickLabel;
    std::shared_ptr<UILabel> _rightStickLabel;
    std::shared_ptr<UIImageView> _holdIconView;
    std::shared_ptr<UILabel> _holdStatusLabel;
    std::shared_ptr<TouchOnlyButton> _exitButton;
    std::vector<EventIcon> _eventIcons;
    std::optional<NXControllerType> _renderedControllerType;

    std::unique_ptr<CADisplayLink> _displayLink;
    std::mutex _inputMutex;
    std::vector<NXActionButton> _pendingButtons;
    std::set<std::pair<SDL_JoystickID, int>> _buttonsDown;
    std::map<SDL_JoystickID, Clock::time_point> _backButtonHolds;
    SDL_JoystickID _activeController = 0;
    NXPoint _leftStickPosition;
    NXPoint _rightStickPosition;
    float _leftTriggerValue = 0;
    float _rightTriggerValue = 0;
    bool _watchInstalled = false;
    bool _exitRequested = false;
};

} // namespace

ControllerTestViewController::ControllerTestViewController() {
    setTitle("Controller");
}

void ControllerTestViewController::loadView() {
    auto rootView = new_shared<UIView>();
    rootView->setBackgroundColor(UIColor::systemBackground);

    auto titleLabel = new_shared<UILabel>();
    titleLabel->setText("Controller input");
    titleLabel->setFontSize(30);
    titleLabel->setFontWeight(600);
    titleLabel->setAutolayoutEnabled(true);

    auto detailLabel = new_shared<UILabel>();
    detailLabel->setText(
        "Inspect every controller button, both sticks, and analog trigger pressure."
    );
    detailLabel->setFontSize(18);
    detailLabel->setTextColor(UIColor::secondaryLabel);
    detailLabel->setAutolayoutEnabled(true);

    auto testButton = new_shared<UIButton>(UIButtonStyle::tinted);
    testButton->setText("Test Controller");
    testButton->layer()->setCornerRadius(10);
    testButton->setAutolayoutEnabled(true);
    testButton->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(100_percent);
        layout->setHeight(64_pt);
        layout->setPaddingHorizontal(20_pt);
    });
    const auto weakSelf = weak_from_base<ControllerTestViewController>();
    testButton->primaryAction = UIAction("Test Controller", [weakSelf]() {
        if (const auto self = weakSelf.lock()) {
            self->show(new_shared<ControllerInputTestViewController>(), self);
        }
    });

    auto contentView = new_shared<UIView>();
    contentView->setAutolayoutEnabled(true);
    contentView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(100_percent);
        layout->setFlexDirection(YGFlexDirectionColumn);
        layout->setAllGap(18);
        layout->setPaddingHorizontal(48_pt);
        layout->setPaddingVertical(48_pt);
    });
    contentView->addSubview(titleLabel);
    contentView->addSubview(detailLabel);
    contentView->addSubview(testButton);

    rootView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setAlignItems(YGAlignCenter);
        layout->setJustifyContent(YGJustifyCenter);
    });
    rootView->addSubview(contentView);
    setView(rootView);
}

void ControllerTestViewController::viewSafeAreaInsetsDidChange() {
    UIViewController::viewSafeAreaInsetsDidChange();
    updateSafeAreaLayout();
}

void ControllerTestViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    updateSafeAreaLayout();
}

void ControllerTestViewController::updateSafeAreaLayout() {
    if (!viewIsLoaded()) {
        return;
    }
    const auto safeArea = resolvedNavigationSafeArea(
        shared_from_base<ControllerTestViewController>(),
        view()
    );
    view()->configureLayout([safeArea](const std::shared_ptr<YGLayout>& layout) {
        layout->setPaddingTop(YGValue {
            static_cast<float>(safeArea.top),
            YGUnitPoint,
        });
        layout->setPaddingLeft(YGValue {
            static_cast<float>(safeArea.left),
            YGUnitPoint,
        });
        layout->setPaddingBottom(YGValue {
            static_cast<float>(safeArea.bottom),
            YGUnitPoint,
        });
        layout->setPaddingRight(YGValue {
            static_cast<float>(safeArea.right),
            YGUnitPoint,
        });
    });
}
