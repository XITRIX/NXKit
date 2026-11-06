#pragma once

#include <UIView.h>
#include <UIAction.h>
#include <cstdint>
#include <optional>
#include <vector>

namespace NXKit {

// UIKit option-set values. The singular spelling is retained for existing code.
enum class UIControlEvent : std::uint32_t {
    touchDown=1u<<0, touchDownRepeat=1u<<1, touchDragInside=1u<<2,
    touchDragOutside=1u<<3, touchDragEnter=1u<<4, touchDragExit=1u<<5,
    touchUpInside=1u<<6, touchUpOutside=1u<<7, touchCancel=1u<<8,
    valueChanged=1u<<12, primaryActionTriggered=1u<<13, menuActionTriggered=1u<<14,
    editingDidBegin=1u<<16, editingChanged=1u<<17, editingDidEnd=1u<<18,
    editingDidEndOnExit=1u<<19, allTouchEvents=0x00000fff,
    allEditingEvents=0x000f0000, applicationReserved=0x0f000000,
    systemReserved=0xf0000000, allEvents=0xffffffff
};
using UIControlEvents = UIControlEvent;
enum class UIControlState : std::uint32_t {
    normal=0, highlighted=1u<<0, disabled=1u<<1, selected=1u<<2, focused=1u<<3,
    application=0x00ff0000, reserved=0xff000000
};
#define NXKIT_CONTROL_OPTIONS(Type) \
constexpr Type operator|(Type a,Type b){return Type(std::uint32_t(a)|std::uint32_t(b));} \
constexpr Type operator&(Type a,Type b){return Type(std::uint32_t(a)&std::uint32_t(b));} \
constexpr Type operator~(Type a){return Type(~std::uint32_t(a));} \
constexpr Type& operator|=(Type& a,Type b){return a=a|b;} \
constexpr Type& operator&=(Type& a,Type b){return a=a&b;}
NXKIT_CONTROL_OPTIONS(UIControlEvent)
NXKIT_CONTROL_OPTIONS(UIControlState)
#undef NXKIT_CONTROL_OPTIONS

enum class UIControlContentVerticalAlignment { center, top, bottom, fill };
enum class UIControlContentHorizontalAlignment { center, left, right, fill, leading, trailing };

class UIControlGestureRecognizer;
class UIControl : public UIView {
public:
    static std::shared_ptr<UIView> init() { return new_shared<UIControl>(); }
    explicit UIControl(NXRect frame = {}, std::optional<UIAction> primaryAction = std::nullopt);

    bool canBecomeFocused() override;
    bool allowsActionDispatch() const override;
    void didUpdateFocusIn(UIFocusUpdateContext context, UIFocusAnimationCoordinator* coordinator) override;
    void pressesBegan(std::set<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;
    void pressesEnded(std::set<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;
    void pressesCancelled(std::set<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) override;

    [[nodiscard]] bool isEnabled() const;
    virtual void setEnabled(bool enabled);
    [[nodiscard]] bool isSelected() const;
    virtual void setSelected(bool selected);
    [[nodiscard]] bool isHighlighted() const;
    virtual void setHighlighted(bool highlighted);
    [[nodiscard]] UIControlState state() const;
    [[nodiscard]] bool isTracking() const { return _tracking; }
    [[nodiscard]] bool isTouchInside() const { return _touchInside; }

    [[nodiscard]] UIControlContentVerticalAlignment contentVerticalAlignment() const { return _verticalAlignment; }
    void setContentVerticalAlignment(UIControlContentVerticalAlignment alignment);
    [[nodiscard]] UIControlContentHorizontalAlignment contentHorizontalAlignment() const { return _horizontalAlignment; }
    void setContentHorizontalAlignment(UIControlContentHorizontalAlignment alignment);
    // Leading/trailing currently resolve to left/right: Core has no RTL trait yet.
    [[nodiscard]] UIControlContentHorizontalAlignment effectiveContentHorizontalAlignment() const;

    virtual bool beginTracking(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event);
    virtual bool continueTracking(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event);
    virtual void endTracking(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event);
    virtual void cancelTracking(const std::shared_ptr<UIEvent>& event = nullptr);

    void addAction(const UIAction& action, UIControlEvent forControlEvents);
    void removeAction(const UIAction& action, UIControlEvent forControlEvents);
    void removeAction(const UIActionIdentifier& identifier, UIControlEvent forControlEvents);
    // Selectors are strings dispatched through UIResponder::performAction.
    // Targets are weak; nullptr searches the responder chain at dispatch time.
    void addTarget(const std::shared_ptr<UIResponder>& target, std::string action, UIControlEvent forControlEvents);
    void removeTarget(const std::shared_ptr<UIResponder>& target, std::optional<std::string> action,
                      UIControlEvent forControlEvents);
    [[nodiscard]] std::vector<std::shared_ptr<UIResponder>> allTargets() const;
    [[nodiscard]] UIControlEvent allControlEvents() const;
    [[nodiscard]] std::vector<std::string> actionsForTarget(const std::shared_ptr<UIResponder>& target,
                                                         UIControlEvent forControlEvent) const;
    using EventHandlerIterator = std::function<void(const std::optional<UIAction>&,
        const std::shared_ptr<UIResponder>&, const std::optional<std::string>&, UIControlEvent, bool& stop)>;
    void enumerateEventHandlers(const EventHandlerIterator& iterator) const;
    virtual void sendAction(const UIAction& action);
    virtual void sendAction(const std::string& action, const std::shared_ptr<UIResponder>& to,
                            const std::shared_ptr<UIEvent>& forEvent = nullptr);
    void sendActions(UIControlEvent forControlEvents);
    void performPrimaryAction();

    virtual void willGainFocus();
    virtual void willLoseFocus();
    virtual void focusFailedToMove(UIFocusHeading heading);
    virtual void willChangeFocusHighlight(bool highlighted);
    virtual void willChangeHighlight(bool highlighted);
    std::shared_ptr<UIView> hitTest(NXPoint point, UIEvent* withEvent) override;
    void setBaseScaleMultiplier(NXFloat multiplier);
    [[nodiscard]] NXFloat baseScaleMultiplier() const { return _baseScaleMultiplier; }
    virtual void baseScaleMultiplierDidChange() {}
    bool applyXMLAttribute(const std::string& name, const std::string& value) override;

    // Compatibility adapter for the original NXKit property. New code should
    // use addAction(..., UIControlEvent::primaryActionTriggered).
    std::optional<UIAction> primaryAction;
protected:
    virtual void controlStateDidChange() {}
    virtual bool requiresExclusiveTouchTracking() const { return false; }
    // A control may keep its final tracking phase alive while its owned
    // presentation settles. These are internal portable subclass hooks.
    virtual bool deferEndTracking(const std::shared_ptr<UITouch>&, const std::shared_ptr<UIEvent>&) { return false; }
    virtual void trackingDidEnd() {}
    void completeTracking();
    void interactionDidBecomeUnavailable() override;
private:
    friend class UIControlGestureRecognizer;
    struct Handler {
        std::optional<UIAction> action;
        std::weak_ptr<UIResponder> target;
        bool responderChain = false;
        std::string selector;
        UIControlEvent events{};
    };
    UIControlState _state = UIControlState::normal;
    UIControlContentVerticalAlignment _verticalAlignment = UIControlContentVerticalAlignment::center;
    UIControlContentHorizontalAlignment _horizontalAlignment = UIControlContentHorizontalAlignment::center;
    NXFloat _baseScaleMultiplier = 1;
    std::vector<Handler> _handlers;
    std::weak_ptr<UIControlGestureRecognizer> _controlGesture;
    std::shared_ptr<UITouch> _trackingTouch;
    std::shared_ptr<UIEvent> _trackingEvent;
    bool _tracking = false, _touchInside = false, _selectPressActive = false;
    std::uint64_t _trackingGeneration = 0;
    bool beginTouch(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event);
    void moveTouch(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event);
    void endTouch(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event, bool allowDeferral = true);
    void clearTracking();
    void dispatch(UIControlEvent events, const std::shared_ptr<UIEvent>& event);
};
}
