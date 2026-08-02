#pragma once

#include <NXResponderAction.h>
#include <UIView.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace NXKit {

class CADisplayLink;
class UIImage;
class UILabel;
class NXNavigationController;

enum class NXNavigationWidgetPlacement {
    hidden,
    navigationLeading,
    navigationTrailing,
    toolbarLeading,
    toolbarTrailing,
};

class NXNavigationStatusView : public UIView {
public:
    NXNavigationStatusView();
    ~NXNavigationStatusView() override;

    [[nodiscard]] bool compact() const { return _compact; }
    void setCompact(bool compact);
    void refresh();

    [[nodiscard]] int batteryLevel() const { return _batteryLevel; }
    [[nodiscard]] std::string timeText() const;

private:
    bool _compact = true;
    int _batteryLevel = -1;
    long long _lastClockSecond = -1;
    long long _lastPowerMinute = -1;
    std::shared_ptr<UIView> _batteryGroup;
    std::shared_ptr<UIView> _batteryBody;
    std::shared_ptr<UIView> _batteryFill;
    std::shared_ptr<UILabel> _batteryPercentageLabel;
    std::shared_ptr<UILabel> _timeLabel;
    std::unique_ptr<CADisplayLink> _displayLink;

    void rebuildLayout();
    void updateBatteryLevel(long long currentMinute);
};

class NXNavigationActionsView : public UIView {
public:
    NXNavigationActionsView();
    ~NXNavigationActionsView() override;

    [[nodiscard]] const std::vector<NXResponderAction>& actions() const { return _actions; }
    void setActions(std::vector<NXResponderAction> actions);
    void setActionProvider(std::function<std::vector<NXResponderAction>()> provider);
    void refresh();

private:
    std::vector<NXResponderAction> _actions;
    std::function<std::vector<NXResponderAction>()> _actionProvider;
    std::unique_ptr<CADisplayLink> _displayLink;
    void performAction(NXActionButton button);
    void rebuildActions();
};

class NXNavigationItem {
public:
    [[nodiscard]] const std::optional<std::string>& titleOverride() const {
        return _titleOverride;
    }
    void setTitleOverride(std::optional<std::string> title);

    [[nodiscard]] std::shared_ptr<UIImage> icon() const { return _icon; }
    void setIcon(std::shared_ptr<UIImage> icon);

    [[nodiscard]] NXNavigationWidgetPlacement statusWidgetPlacement() const {
        return _statusWidgetPlacement;
    }
    void setStatusWidgetPlacement(NXNavigationWidgetPlacement placement);

    [[nodiscard]] NXNavigationWidgetPlacement actionsWidgetPlacement() const {
        return _actionsWidgetPlacement;
    }
    void setActionsWidgetPlacement(NXNavigationWidgetPlacement placement);

    // A null custom widget uses the navigation controller's live battery/time widget.
    [[nodiscard]] std::shared_ptr<UIView> statusWidget() const { return _statusWidget; }
    void setStatusWidget(std::shared_ptr<UIView> widget);

    // A null custom widget uses the navigation controller's action-hint widget.
    [[nodiscard]] std::shared_ptr<UIView> actionsWidget() const { return _actionsWidget; }
    void setActionsWidget(std::shared_ptr<UIView> widget);

private:
    friend class NXNavigationController;

    std::optional<std::string> _titleOverride;
    std::shared_ptr<UIImage> _icon;
    NXNavigationWidgetPlacement _statusWidgetPlacement =
        NXNavigationWidgetPlacement::toolbarLeading;
    NXNavigationWidgetPlacement _actionsWidgetPlacement =
        NXNavigationWidgetPlacement::toolbarTrailing;
    std::shared_ptr<UIView> _statusWidget;
    std::shared_ptr<UIView> _actionsWidget;
    std::function<void()> _changeHandler;

    void notifyChanged();
};

}
