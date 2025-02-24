#pragma once

#include <Geometry.h>
#include <UIGestureRecognizer.h>
#include <Timer.h>
#include <vector>
#include <ctime>

namespace NXKit {

enum class UITouchPhase {
    began, moved, ended
};

class UIView;
class UIWindow;
class UITouch {
public:
    UITouch(): UITouch(0, NXPoint(), Timer()) {}
    UITouch(unsigned long touchId, NXPoint point, Timer timestamp);

    [[nodiscard]] constexpr unsigned long touchId() const { return _touchId; }

    [[nodiscard]] std::weak_ptr<UIView> view() const { return _view; }
    [[nodiscard]] std::weak_ptr<UIWindow> window() const { return _window; }

    [[nodiscard]] UITouchPhase phase() const { return _phase; }
    [[nodiscard]] Timer timestamp() const { return _timestamp; }

    [[nodiscard]] NXPoint absoluteLocation() const { return _absoluteLocation; }
    [[nodiscard]] NXPoint previousAbsoluteLocation() const { return _previousAbsoluteLocation; }

    std::vector<std::weak_ptr<UIGestureRecognizer>> gestureRecognizers() { return _gestureRecognizers; };

    void updateAbsoluteLocation(NXPoint newLocation);

    NXPoint locationIn(std::shared_ptr<UIView> view);
    NXPoint previousLocationIn(std::shared_ptr<UIView> view);

    void runTouchActionOnRecognizerHierachy(const std::function<void(std::shared_ptr<UIGestureRecognizer>)>& action);

    [[nodiscard]] bool hasBeenCancelledByAGestureRecognizer() const { return _hasBeenCancelledByAGestureRecognizer; }

private:
    unsigned long _touchId;

    std::weak_ptr<UIView> _view;
    std::weak_ptr<UIWindow> _window;

    UITouchPhase _phase = UITouchPhase::began;
    Timer _timestamp;

    NXPoint _absoluteLocation;
    NXPoint _previousAbsoluteLocation;

    std::vector<std::weak_ptr<UIGestureRecognizer>> _gestureRecognizers;
    bool _hasBeenCancelledByAGestureRecognizer = false;

    friend class UIApplication;
    friend class UIWindow;
    friend class UIGestureRecognizer;
};

constexpr bool operator==(const UITouch& lhs, const UITouch& rhs) {
    return lhs.touchId() == rhs.touchId();
}

}

