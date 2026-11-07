#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <cstdint>
#include <optional>
#include <tools/SharedBase.hpp>

namespace NXKit {

class UITouch;
class UIPress;
class UIPressesEvent;

enum class UIGestureRecognizerState {
    possible,
    began,
    changed,
    ended,
    cancelled,
    failed,
    recognized = ended
};

class UIView;
class UIEvent;
class UIGestureRecognizer;
class UIGestureRecognizerDelegate {
public:
    virtual ~UIGestureRecognizerDelegate() = default;
    virtual bool gestureRecognizerShouldBegin(std::shared_ptr<UIGestureRecognizer>) { return true; }
    virtual bool gestureRecognizerShouldRecognizeSimultaneouslyWith(std::shared_ptr<UIGestureRecognizer> gestureRecognizer, std::shared_ptr<UIGestureRecognizer> otherGestureRecognizer) { return false; }
    virtual bool gestureRecognizerShouldRequireFailureOf(std::shared_ptr<UIGestureRecognizer>, std::shared_ptr<UIGestureRecognizer>) { return false; }
    virtual bool gestureRecognizerShouldBeRequiredToFailBy(std::shared_ptr<UIGestureRecognizer>, std::shared_ptr<UIGestureRecognizer>) { return false; }
    virtual bool gestureRecognizerShouldReceiveTouch(std::shared_ptr<UIGestureRecognizer>, std::shared_ptr<UITouch>) { return true; }
    virtual bool gestureRecognizerShouldReceivePress(std::shared_ptr<UIGestureRecognizer>, std::shared_ptr<UIPress>) { return true; }
    virtual bool gestureRecognizerShouldReceiveEvent(std::shared_ptr<UIGestureRecognizer>, std::shared_ptr<UIEvent>) { return true; }
};

class UIGestureRecognizer: public enable_shared_from_this<UIGestureRecognizer> {
public:
    std::weak_ptr<UIGestureRecognizerDelegate> delegate;
    std::function<void(std::shared_ptr<UIGestureRecognizer>)> onStateChanged = [](auto self){};

    UIGestureRecognizer(std::function<void(std::shared_ptr<UIGestureRecognizer>)> onStateChanged = [](auto) {});
    virtual ~UIGestureRecognizer();

    bool isEnabled() { return _isEnabled; }
    void setEnabled(bool enabled);

    UIGestureRecognizerState state() { return _state; }
    [[nodiscard]] virtual std::size_t numberOfTouches() const;
    void setState(UIGestureRecognizerState state);
    // Failure requirements do not retain either recognizer. Invalid or cyclic
    // explicit requirements throw std::invalid_argument.
    void requireGestureRecognizerToFail(const std::shared_ptr<UIGestureRecognizer>& otherGestureRecognizer);
    virtual bool shouldRequireFailureOf(const std::shared_ptr<UIGestureRecognizer>& otherGestureRecognizer) const;
    virtual bool shouldBeRequiredToFailBy(const std::shared_ptr<UIGestureRecognizer>& otherGestureRecognizer) const;

    std::weak_ptr<UIView> view() { return _view; }

    bool cancelsTouchesInView() const { return _cancelsTouchesInView; }
    void setCancelsTouchesInView(bool cancelsTouchesInView) {
        _cancelsTouchesInView = cancelsTouchesInView;
    }

    // These flags currently delay UIView touch responder delivery. Delaying
    // hardware/controller press responder delivery is not implemented yet.
    bool delaysTouchesBegan() const { return _delaysTouchesBegan; }
    void setDelaysTouchesBegan(bool value);
    bool delaysTouchesEnded() const { return _delaysTouchesEnded; }
    void setDelaysTouchesEnded(bool value);

    virtual bool canPrevent(
        const std::shared_ptr<UIGestureRecognizer>& preventedGestureRecognizer
    ) const;
    virtual bool canBePreventedBy(
        const std::shared_ptr<UIGestureRecognizer>& preventingGestureRecognizer
    ) const;

    virtual void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    virtual void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    virtual void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    virtual void touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);

    virtual void pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);
    virtual void pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);
    virtual void pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);
    virtual void pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);

protected:
    virtual void reset();
    // Framework recognizers retain their action when applications observe the
    // public callback. Ordinary recognizers only deliver onStateChanged.
    virtual void didChangeState() {}
    void notifyStateChanged();

    UIGestureRecognizerState _state = UIGestureRecognizerState::possible;
    
private:
    bool _isEnabled = true;
    bool _cancelsTouchesInView = true;
    bool _delaysTouchesBegan = false;
    bool _delaysTouchesEnded = true;
    bool _didResetCurrentAttempt = false;
    std::uint64_t _resetGeneration = 0;
    std::weak_ptr<UIView> _view;
    std::vector<std::shared_ptr<UITouch>> _allTouches;
    std::vector<std::shared_ptr<UIPress>> _allPresses;
    std::vector<std::weak_ptr<UITouch>> _terminalTouches;
    std::vector<std::weak_ptr<UIPress>> _terminalPresses;
    std::vector<std::weak_ptr<UITouch>> _ignoredTouches;
    std::vector<std::weak_ptr<UIPress>> _ignoredPresses;
    std::vector<std::weak_ptr<UIGestureRecognizer>> _requiredToFail;
    std::vector<std::weak_ptr<UIGestureRecognizer>> _attemptRequirements;
    std::optional<UIGestureRecognizerState> _pendingRecognition;
    bool _requirementsResolved = false;
    bool _askedShouldBegin = false;
    bool _needsAttemptCleanup = false;
    bool _attemptCancelled = false;

    void commitState(UIGestureRecognizerState state);
    bool resolvePendingRecognition();
    void prepareFailureRequirements();
    bool requiresFailureTransitively(const UIGestureRecognizer* other) const;
    bool sharesAttemptWith(const UIGestureRecognizer& other) const;
    bool hasLiveInputs() const;
    void registerAttempt();
    void finishAttempt();
    void cancelForUnavailableView();
    static void beginEventDispatch();
    static void endEventDispatch();
    static void resolvePendingRecognitions();
    static void flushResponderTouchDelivery();

    void _touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    void _touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    void _touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    void _touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);

    void _pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);
    void _pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);
    void _pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);
    void _pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);

    bool recognitionCondition();

    void addTouch(std::shared_ptr<UITouch> touch);
    void removeTouch(std::shared_ptr<UITouch> touch);
    void resetCurrentAttemptIfNeeded();
    void cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously();

    friend class UIView;
    friend class UIWindow;
    friend class UIGestureRecognizerTestHarness;
};

}
