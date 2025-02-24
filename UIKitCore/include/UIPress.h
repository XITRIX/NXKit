#pragma once

#include <tools/Tools.hpp>
#include <UIKey.h>
#include <UIGestureRecognizer.h>
#include <Timer.h>
#include <ctime>
#include <optional>

namespace NXKit {

enum class UIPressPhase {
    began, changed, stationary, ended
};

enum class UIPressType {
    none,
    upArrow,
    downArrow,
    leftArrow,
    rightArrow,
    select,
    menu,
    playPause,
};

class UIResponder;
class UIWindow;
class UIPress {
public:
    explicit UIPress(Timer timestamp = Timer());

    [[nodiscard]] Timer timestamp() const { return _timestamp; }
    [[nodiscard]] UIPressPhase phase() const { return _phase; }
    [[nodiscard]] UIPressType type() const { return _type; }

    [[nodiscard]] std::weak_ptr<UIWindow> window() const { return _window; }
    [[nodiscard]] std::weak_ptr<UIResponder> responder() const { return _responder; }

    std::vector<std::weak_ptr<UIGestureRecognizer>> gestureRecognizers() { return _gestureRecognizers; };
    std::optional<UIKey> key() { return _key; }

private:
    std::weak_ptr<UIResponder> _responder;
    std::weak_ptr<UIWindow> _window;

    UIPressPhase _phase = UIPressPhase::began;
    UIPressType _type = UIPressType::none;
    Timer _timestamp;

    std::vector<std::weak_ptr<UIGestureRecognizer>> _gestureRecognizers;
    bool _hasBeenCancelledByAGestureRecognizer = false;
    std::optional<UIKey> _key;

    void setForWindow(const std::shared_ptr<UIWindow>& window);
    void runPressActionOnRecognizerHierachy(const std::function<void(std::shared_ptr<UIGestureRecognizer>)>& action);
    [[nodiscard]] bool hasBeenCancelledByAGestureRecognizer() const { return _hasBeenCancelledByAGestureRecognizer; }

    friend class UIApplication;
    friend class UIGestureRecognizer;
    friend class UIWindow;
};

}

