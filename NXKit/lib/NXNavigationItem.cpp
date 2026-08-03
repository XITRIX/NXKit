#include <NXNavigationItem.h>

#include <CADisplayLink.h>
#include <UIControl.h>
#include <UILabel.h>

#include <SDL3/SDL_power.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <utility>

using namespace NXKit;
using namespace NXKit::yoga::literals;

namespace {

constexpr NXFloat batteryBodyWidth = 36;
constexpr NXFloat batteryBodyHeight = 22;
constexpr NXFloat batteryBorderWidth = 6;
constexpr NXFloat batteryContentInset = 5;
constexpr NXFloat batteryFillWidth = batteryBodyWidth - batteryContentInset * 2;
constexpr NXFloat batteryFillHeight = batteryBodyHeight - batteryContentInset * 2;
constexpr NXFloat batteryTipWidth = 4;
constexpr NXFloat batteryTipHeight = 10;

std::string actionButtonTitle(NXActionButton button) {
    switch (button) {
        case NXActionButton::a: return "A";
        case NXActionButton::b: return "B";
        case NXActionButton::x: return "X";
        case NXActionButton::y: return "Y";
        case NXActionButton::plus: return "+";
        case NXActionButton::minus: return "-";
    }
    return "";
}

class NXNavigationActionView final : public UIControl {
public:
    NXNavigationActionView(
        const NXResponderAction& action,
        std::function<void()> touchHandler
    ) {
        setAutolayoutEnabled(true);
        const bool isTouchEnabled = action.button != NXActionButton::a
            && action.isEnabled;
        setUserInteractionEnabled(isTouchEnabled);
        if (isTouchEnabled) {
            primaryAction = UIAction(action.action.title(), std::move(touchHandler));
        }
        configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setFlexDirection(YGFlexDirectionRow);
            layout->setAlignItems(YGAlignCenter);
            layout->setAllGap(8);
        });

        auto buttonCircle = new_shared<UIView>();
        buttonCircle->setAutolayoutEnabled(true);
        buttonCircle->setUserInteractionEnabled(false);
        buttonCircle->setBackgroundColor(
            action.isEnabled ? UIColor::label : UIColor::tertiaryLabel
        );
        buttonCircle->layer()->setCornerRadius(12);
        buttonCircle->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setSize({24, 24});
            layout->setAlignItems(YGAlignCenter);
            layout->setJustifyContent(YGJustifyCenter);
        });

        auto buttonLabel = new_shared<UILabel>();
        buttonLabel->setAutolayoutEnabled(true);
        buttonLabel->setUserInteractionEnabled(false);
        buttonLabel->setText(actionButtonTitle(action.button));
        buttonLabel->setFontSize(15);
        buttonLabel->setFontWeight(700);
        buttonLabel->setTextAlignment(NSTextAlignment::center);
        buttonLabel->setTextColor(
            action.isEnabled ? UIColor::systemBackground : UIColor::systemGray5
        );
        buttonLabel->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setWidth(24_pt);
        });
        buttonCircle->addSubview(buttonLabel);

        auto titleLabel = new_shared<UILabel>();
        titleLabel->setAutolayoutEnabled(true);
        titleLabel->setUserInteractionEnabled(false);
        titleLabel->setText(action.action.title());
        titleLabel->setFontSize(22);
        titleLabel->setTextColor(
            action.isEnabled ? UIColor::label : UIColor::tertiaryLabel
        );

        addSubview(buttonCircle);
        addSubview(titleLabel);
    }

    bool canBecomeFocused() override {
        return false;
    }
};

std::tm localTime(std::time_t time) {
    std::tm result {};
#if defined(_WIN32)
    localtime_s(&result, &time);
#else
    localtime_r(&time, &result);
#endif
    return result;
}

}

NXNavigationStatusView::NXNavigationStatusView() {
    setAutolayoutEnabled(true);
    setUserInteractionEnabled(false);
    configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setAlignItems(YGAlignCenter);
        layout->setAllGap(30);
    });

    _batteryGroup = new_shared<UIView>();
    _batteryGroup->setAutolayoutEnabled(true);
    _batteryGroup->setUserInteractionEnabled(false);
    _batteryGroup->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setAlignItems(YGAlignCenter);
        layout->setAllGap(0);
        layout->setWidth(YGValue {
            static_cast<float>(batteryBodyWidth + batteryTipWidth),
            YGUnitPoint
        });
        layout->setHeight(YGValue { static_cast<float>(batteryBodyHeight), YGUnitPoint });
    });

    _batteryBody = new_shared<UIView>();
    _batteryBody->setAutolayoutEnabled(true);
    _batteryBody->setUserInteractionEnabled(false);
    _batteryBody->setClipsToBounds(true);
    _batteryBody->layer()->setBorderColor(UIColor::label);
    _batteryBody->layer()->setBorderWidth(batteryBorderWidth);
    _batteryBody->layer()->setCornerRadius(1.5);
    _batteryBody->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(YGValue { static_cast<float>(batteryBodyWidth), YGUnitPoint });
        layout->setHeight(YGValue { static_cast<float>(batteryBodyHeight), YGUnitPoint });
    });

    _batteryFill = new_shared<UIView>();
    _batteryFill->setAutolayoutEnabled(true);
    _batteryFill->setUserInteractionEnabled(false);
    _batteryFill->setBackgroundColor(UIColor::label);
    _batteryFill->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setPositionType(YGPositionTypeAbsolute);
        layout->setLeft(YGValue { static_cast<float>(batteryContentInset), YGUnitPoint });
        layout->setTop(YGValue { static_cast<float>(batteryContentInset), YGUnitPoint });
        layout->setWidth(YGValue { static_cast<float>(batteryFillWidth), YGUnitPoint });
        layout->setHeight(YGValue { static_cast<float>(batteryFillHeight), YGUnitPoint });
    });
    _batteryBody->addSubview(_batteryFill);

    auto batteryTip = new_shared<UIView>();
    batteryTip->setAutolayoutEnabled(true);
    batteryTip->setUserInteractionEnabled(false);
    batteryTip->setBackgroundColor(UIColor::label);
    batteryTip->layer()->setCornerRadius(1);
    batteryTip->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(YGValue { static_cast<float>(batteryTipWidth), YGUnitPoint });
        layout->setHeight(YGValue { static_cast<float>(batteryTipHeight), YGUnitPoint });
    });
    _batteryGroup->addSubview(_batteryBody);
    _batteryGroup->addSubview(batteryTip);

    _batteryPercentageLabel = new_shared<UILabel>();
    _batteryPercentageLabel->setAutolayoutEnabled(true);
    _batteryPercentageLabel->setUserInteractionEnabled(false);
    _batteryPercentageLabel->setFontSize(21);

    _timeLabel = new_shared<UILabel>();
    _timeLabel->setAutolayoutEnabled(true);
    _timeLabel->setUserInteractionEnabled(false);
    _timeLabel->setFontSize(21);
    _timeLabel->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexShrink(0);
    });

    rebuildLayout();
    refresh();

    const auto weakSelf = weak_from_base<NXNavigationStatusView>();
    _displayLink = std::make_unique<CADisplayLink>([weakSelf]() {
        if (const auto self = weakSelf.lock()) {
            self->refresh();
        }
    });
}

NXNavigationStatusView::~NXNavigationStatusView() = default;

void NXNavigationStatusView::setCompact(bool compact) {
    if (_compact == compact) {
        return;
    }
    _compact = compact;
    _lastClockSecond = -1;
    configureLayout([compact](const std::shared_ptr<YGLayout>& layout) {
        layout->setAllGap(compact ? 30 : 14);
    });
    rebuildLayout();
    refresh();
}

void NXNavigationStatusView::rebuildLayout() {
    const auto previousSubviews = subviews();
    for (const auto& subview : previousSubviews) {
        subview->removeFromSuperview();
    }

    if (_compact) {
        addSubview(_batteryGroup);
        addSubview(_timeLabel);
    } else {
        addSubview(_timeLabel);
        addSubview(_batteryPercentageLabel);
        addSubview(_batteryGroup);
    }
}

void NXNavigationStatusView::updateBatteryLevel(long long currentMinute) {
    if (_lastPowerMinute == currentMinute) {
        return;
    }
    _lastPowerMinute = currentMinute;

    int percent = -1;
    SDL_GetPowerInfo(nullptr, &percent);
    _batteryLevel = std::clamp(percent, -1, 100);

    const auto fillWidth = _batteryLevel < 0
        ? 0.0f
        : static_cast<float>(batteryFillWidth) * static_cast<float>(_batteryLevel) / 100.0f;
    _batteryFill->configureLayout([fillWidth](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(YGValue { fillWidth, YGUnitPoint });
    });
    _batteryPercentageLabel->setText(
        _batteryLevel < 0 ? "--%" : std::to_string(_batteryLevel) + "%"
    );
}

void NXNavigationStatusView::refresh() {
    const auto now = std::chrono::system_clock::now();
    const auto currentSecond = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()
    ).count();
    if (_lastClockSecond == currentSecond) {
        return;
    }
    _lastClockSecond = currentSecond;

    const auto time = std::chrono::system_clock::to_time_t(now);
    const auto components = localTime(time);
    std::ostringstream stream;
    stream << std::put_time(&components, _compact ? "%H:%M:%S" : "%H:%M");
    _timeLabel->setText(stream.str());
    updateBatteryLevel(currentSecond / 60);
}

std::string NXNavigationStatusView::timeText() const {
    return _timeLabel ? _timeLabel->text() : "";
}

NXNavigationActionsView::NXNavigationActionsView() {
    setAutolayoutEnabled(true);
    setUserInteractionEnabled(true);
    configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setAlignItems(YGAlignCenter);
        layout->setAllGap(30);
    });
    setActions({});
}

NXNavigationActionsView::~NXNavigationActionsView() = default;

void NXNavigationActionsView::setActions(std::vector<NXResponderAction> actions) {
    std::unordered_set<NXActionButton> buttons;
    for (const auto& action : actions) {
        if (!buttons.insert(action.button).second) {
            throw std::invalid_argument(
                "NXNavigationActionsView does not accept duplicate action buttons"
            );
        }
    }

    if (!buttons.contains(NXActionButton::a)) {
        actions.push_back(NXResponderAction {
            .button = NXActionButton::a,
            .isEnabled = false,
            .action = UIAction("OK"),
        });
    }
    if (_actions == actions) {
        // The visible metadata can stay identical while responder registration
        // replaces the executable callback. Keep the latest resolved actions
        // without rebuilding the legend every display-link tick.
        _actions = std::move(actions);
        return;
    }
    _actions = std::move(actions);
    rebuildActions();
}

void NXNavigationActionsView::setActionProvider(
    std::function<std::vector<NXResponderAction>()> provider
) {
    _actionProvider = std::move(provider);
    if (_actionProvider && !_displayLink) {
        const auto weakSelf = weak_from_base<NXNavigationActionsView>();
        _displayLink = std::make_unique<CADisplayLink>([weakSelf]() {
            if (const auto self = weakSelf.lock()) {
                self->refresh();
            }
        });
    } else if (!_actionProvider) {
        _displayLink.reset();
    }
    refresh();
}

void NXNavigationActionsView::refresh() {
    if (_actionProvider) {
        setActions(_actionProvider());
    }
}

void NXNavigationActionsView::performAction(NXActionButton button) {
    if (button == NXActionButton::a) {
        return;
    }
    const auto match = std::find_if(
        _actions.begin(),
        _actions.end(),
        [button](const NXResponderAction& action) {
            return action.button == button;
        }
    );
    if (match == _actions.end() || !match->isEnabled) {
        return;
    }

    // The callback may mutate the responder chain and rebuild this widget.
    const auto action = match->action;
    action.perform();
}

void NXNavigationActionsView::rebuildActions() {
    const auto previousSubviews = subviews();
    for (const auto& subview : previousSubviews) {
        subview->removeFromSuperview();
    }
    for (const auto& action : _actions) {
        const auto button = action.button;
        const auto weakSelf = weak_from_base<NXNavigationActionsView>();
        addSubview(new_shared<NXNavigationActionView>(
            action,
            [weakSelf, button]() {
                if (const auto self = weakSelf.lock()) {
                    self->performAction(button);
                }
            }
        ));
    }
}

void NXNavigationItem::setTitleOverride(std::optional<std::string> title) {
    if (_titleOverride == title) {
        return;
    }
    _titleOverride = std::move(title);
    notifyChanged();
}

void NXNavigationItem::setIcon(std::shared_ptr<UIImage> icon) {
    if (_icon == icon) {
        return;
    }
    _icon = std::move(icon);
    notifyChanged();
}

void NXNavigationItem::setStatusWidgetPlacement(NXNavigationWidgetPlacement placement) {
    if (_statusWidgetPlacement == placement) {
        return;
    }
    _statusWidgetPlacement = placement;
    notifyChanged();
}

void NXNavigationItem::setActionsWidgetPlacement(NXNavigationWidgetPlacement placement) {
    if (_actionsWidgetPlacement == placement) {
        return;
    }
    _actionsWidgetPlacement = placement;
    notifyChanged();
}

void NXNavigationItem::setStatusWidget(std::shared_ptr<UIView> widget) {
    if (_statusWidget == widget) {
        return;
    }
    _statusWidget = std::move(widget);
    notifyChanged();
}

void NXNavigationItem::setActionsWidget(std::shared_ptr<UIView> widget) {
    if (_actionsWidget == widget) {
        return;
    }
    _actionsWidget = std::move(widget);
    notifyChanged();
}

void NXNavigationItem::notifyChanged() {
    if (_changeHandler) {
        _changeHandler();
    }
}
