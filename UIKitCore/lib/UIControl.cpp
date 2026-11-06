#include <UIControl.h>
#include <UIControlGestureRecognizer.h>
#include <UITouch.h>
#include <UIPress.h>
#include <UIWindow.h>
#include <tools/IBTools.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_set>

namespace NXKit {
namespace {
bool contains(UIControlEvent a, UIControlEvent b) { return std::uint32_t(a & b) != 0; }
bool contains(UIControlState a, UIControlState b) { return std::uint32_t(a & b) != 0; }
}
UIControl::UIControl(NXRect frame, std::optional<UIAction> action) : UIView(frame) {
    if (action) addAction(*action,UIControlEvent::primaryActionTriggered);
    auto gesture = new_shared<UIControlGestureRecognizer>(weak_from_base<UIControl>());
    _controlGesture = gesture;
    addGestureRecognizer(gesture);
}
bool UIControl::canBecomeFocused() { return isEnabled() && isActionDispatchAllowed(); }
bool UIControl::allowsActionDispatch() const { return isEnabled() && UIView::allowsActionDispatch(); }
bool UIControl::isEnabled() const { return !contains(_state, UIControlState::disabled); }
bool UIControl::isSelected() const { return contains(_state, UIControlState::selected); }
bool UIControl::isHighlighted() const { return contains(_state, UIControlState::highlighted); }
UIControlState UIControl::state() const {
    return _state | (const_cast<UIControl*>(this)->isFocused() ? UIControlState::focused : UIControlState::normal);
}
void UIControl::setEnabled(bool enabled) {
    if (enabled == isEnabled()) return;
    if (enabled) _state &= ~UIControlState::disabled; else _state |= UIControlState::disabled;
    if (!enabled) {
        cancelTracking();
        if (_selectPressActive) { _selectPressActive=false; setHighlighted(false); }
    }
    setNeedsDisplay(); setNeedsLayout(); controlStateDidChange();
}
void UIControl::setSelected(bool selected) {
    if (selected == isSelected()) return;
    if (selected) _state |= UIControlState::selected; else _state &= ~UIControlState::selected;
    setNeedsDisplay(); setNeedsLayout(); controlStateDidChange();
}
void UIControl::setHighlighted(bool highlighted) {
    if (highlighted == isHighlighted()) return;
    if (highlighted) _state |= UIControlState::highlighted; else _state &= ~UIControlState::highlighted;
    setNeedsDisplay(); controlStateDidChange();
    const bool focused = isFocused();
    UIView::animate(.2, 0, UIViewAnimationOptions::allowUserInteraction, [&] {
        if (focused) willChangeFocusHighlight(highlighted); else willChangeHighlight(highlighted);
    });
}
void UIControl::setContentVerticalAlignment(UIControlContentVerticalAlignment value) {
    if (value < UIControlContentVerticalAlignment::center || value > UIControlContentVerticalAlignment::fill)
        throw std::invalid_argument("Invalid control vertical alignment");
    if (_verticalAlignment == value) return;
    _verticalAlignment=value; setNeedsLayout();
}
void UIControl::setContentHorizontalAlignment(UIControlContentHorizontalAlignment value) {
    if (value < UIControlContentHorizontalAlignment::center || value > UIControlContentHorizontalAlignment::trailing)
        throw std::invalid_argument("Invalid control horizontal alignment");
    if (_horizontalAlignment == value) return;
    _horizontalAlignment=value; setNeedsLayout();
}
UIControlContentHorizontalAlignment UIControl::effectiveContentHorizontalAlignment() const {
    if (_horizontalAlignment == UIControlContentHorizontalAlignment::leading) return UIControlContentHorizontalAlignment::left;
    if (_horizontalAlignment == UIControlContentHorizontalAlignment::trailing) return UIControlContentHorizontalAlignment::right;
    return _horizontalAlignment;
}
bool UIControl::beginTracking(const std::shared_ptr<UITouch>&, const std::shared_ptr<UIEvent>&) { return true; }
bool UIControl::continueTracking(const std::shared_ptr<UITouch>&, const std::shared_ptr<UIEvent>&) { return true; }
void UIControl::endTracking(const std::shared_ptr<UITouch>&, const std::shared_ptr<UIEvent>&) {}
void UIControl::clearTracking() {
    ++_trackingGeneration; _tracking=false; _touchInside=false; _trackingTouch.reset(); _trackingEvent.reset(); setHighlighted(false);
}
void UIControl::cancelTracking(const std::shared_ptr<UIEvent>& event) {
    if (!_tracking) return;
    clearTracking(); dispatch(UIControlEvent::touchCancel, event);
}
bool UIControl::beginTouch(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event) {
    if (!touch || _tracking || !isEnabled() || !isActionDispatchAllowed()) return false;
    _tracking=true; _trackingTouch=touch; _trackingEvent=event; _touchInside=point(touch->locationIn(shared_from_this()),event.get());
    const auto generation=++_trackingGeneration;
    const bool accepted=beginTracking(touch,event);
    if (_trackingGeneration!=generation || !_tracking) return false;
    if (!accepted) { clearTracking(); return false; }
    setHighlighted(_touchInside);
    if (_trackingGeneration!=generation || !_tracking) return false;
    dispatch(touch->tapCount()>1?UIControlEvent::touchDown|UIControlEvent::touchDownRepeat:UIControlEvent::touchDown,event);
    return _tracking && _trackingTouch==touch;
}
void UIControl::moveTouch(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event) {
    if (!_tracking || _trackingTouch!=touch) return;
    if (!isActionDispatchAllowed()) { cancelTracking(event); return; }
    const bool wasInside=_touchInside;
    _touchInside=point(touch->locationIn(shared_from_this()),event.get());
    const auto generation=_trackingGeneration;
    const bool keepTracking=continueTracking(touch,event);
    if (_trackingGeneration!=generation || !_tracking) return;
    if (!keepTracking) { clearTracking(); return; }
    setHighlighted(_touchInside);
    if (_trackingGeneration!=generation || !_tracking) return;
    auto events=_touchInside?UIControlEvent::touchDragInside:UIControlEvent::touchDragOutside;
    if (wasInside!=_touchInside) events|=_touchInside?UIControlEvent::touchDragEnter:UIControlEvent::touchDragExit;
    dispatch(events,event);
}
void UIControl::endTouch(const std::shared_ptr<UITouch>& touch, const std::shared_ptr<UIEvent>& event, bool allowDeferral) {
    if (!_tracking || _trackingTouch!=touch) return;
    if (!isActionDispatchAllowed()) { cancelTracking(event); return; }
    _touchInside=point(touch->locationIn(shared_from_this()),event.get());
    _trackingEvent=event;
    if (allowDeferral && deferEndTracking(touch,event)) return;
    if (!_tracking || _trackingTouch!=touch) return;
    const auto generation=_trackingGeneration;
    endTracking(touch,event);
    if (_trackingGeneration!=generation || !_tracking) return;
    const bool inside=_touchInside;
    clearTracking();
    const auto completedGeneration=_trackingGeneration;
    dispatch(inside?UIControlEvent::touchUpInside:UIControlEvent::touchUpOutside,event);
    if (_trackingGeneration==completedGeneration) trackingDidEnd();
    if (inside && isActionDispatchAllowed()) performPrimaryAction();
}
void UIControl::completeTracking() {
    const auto touch=_trackingTouch;
    const auto event=_trackingEvent;
    if (_tracking && touch) endTouch(touch,event,false);
}
void UIControl::interactionDidBecomeUnavailable() {
    cancelTracking(); setHighlighted(false);
    if (auto gesture=_controlGesture.lock()) { const bool enabled=gesture->isEnabled(); gesture->setEnabled(false); gesture->setEnabled(enabled); }
    UIView::interactionDidBecomeUnavailable();
}
void UIControl::addAction(const UIAction& action, UIControlEvent events) {
    if (!std::uint32_t(events)) return;
    for (auto& handler:_handlers) if (handler.action && handler.action->identifier()==action.identifier()) {
        handler.action=action; handler.events|=events; return;
    }
    _handlers.push_back({action,{},false,{},events});
}
void UIControl::removeAction(const UIAction& action, UIControlEvent events) { removeAction(action.identifier(),events); }
void UIControl::removeAction(const UIActionIdentifier& identifier, UIControlEvent events) {
    for (auto& handler:_handlers) if (handler.action && handler.action->identifier()==identifier) handler.events&=~events;
    std::erase_if(_handlers,[](const auto& h){return !std::uint32_t(h.events);});
}
void UIControl::addTarget(const std::shared_ptr<UIResponder>& target, std::string action, UIControlEvent events) {
    if (action.empty()) throw std::invalid_argument("Control action selector cannot be empty");
    if (!std::uint32_t(events)) return;
    for (auto& h:_handlers) if (!h.action && h.responderChain==!target && h.target.lock()==target && h.selector==action) {
        h.events|=events; return;
    }
    _handlers.push_back({{},target,!target,std::move(action),events});
}
void UIControl::removeTarget(const std::shared_ptr<UIResponder>& target, std::optional<std::string> action, UIControlEvent events) {
    for (auto& h:_handlers) if (!h.action && (!target || h.target.lock()==target) && (!action || h.selector==*action)) h.events&=~events;
    std::erase_if(_handlers,[](const auto& h){return !std::uint32_t(h.events);});
}
std::vector<std::shared_ptr<UIResponder>> UIControl::allTargets() const {
    std::vector<std::shared_ptr<UIResponder>> result;
    for (const auto& h:_handlers) if (!h.action) {
        auto target=h.target.lock(); if (!target && !h.responderChain) continue;
        if (std::find(result.begin(),result.end(),target)==result.end()) result.push_back(std::move(target));
    }
    return result;
}
UIControlEvent UIControl::allControlEvents() const {
    auto result=primaryAction?UIControlEvent::primaryActionTriggered:UIControlEvent{};
    for (const auto& h:_handlers) if (h.action || h.responderChain || !h.target.expired()) result|=h.events;
    return result;
}
std::vector<std::string> UIControl::actionsForTarget(const std::shared_ptr<UIResponder>& target, UIControlEvent events) const {
    std::vector<std::string> result;
    for (const auto& h:_handlers) if (!h.action && h.responderChain==!target && h.target.lock()==target && contains(h.events,events)) result.push_back(h.selector);
    return result;
}
void UIControl::enumerateEventHandlers(const EventHandlerIterator& iterator) const {
    if (!iterator) return;
    const auto callback=iterator;
    const auto handlers=_handlers; bool stop=false;
    for (const auto& h:handlers) {
        auto target=h.target.lock(); if (!h.action && !h.responderChain && !target) continue;
        callback(h.action,target,h.action?std::nullopt:std::optional(h.selector),h.events,stop);
        if (stop) break;
    }
}
void UIControl::sendAction(const UIAction& action) {
    auto copy=action; copy._sender=shared_from_base<UIResponder>(); copy.perform();
}
void UIControl::sendAction(const std::string& action, const std::shared_ptr<UIResponder>& to, const std::shared_ptr<UIEvent>& event) {
    const auto sender=shared_from_base<UIResponder>();
    if (to) { to->performAction(action,sender,event); return; }
    std::unordered_set<UIResponder*> visited;
    for (auto target=sender;target && visited.insert(target.get()).second;target=target->next())
        if (target->canPerformAction(action,sender)) { target->performAction(action,sender,event); break; }
}
void UIControl::dispatch(UIControlEvent events, const std::shared_ptr<UIEvent>& event) {
    const auto retained=shared_from_this();
    const auto handlers=_handlers;
    const auto legacy=primaryAction;
    for (const auto& h:handlers) if (contains(h.events,events)) {
        if (h.action) sendAction(*h.action);
        else if (const auto target=h.target.lock();target || h.responderChain) sendAction(h.selector,target,event);
    }
    if (legacy && contains(events,UIControlEvent::primaryActionTriggered)) sendAction(*legacy);
}
void UIControl::sendActions(UIControlEvent events) { dispatch(events,nullptr); }
void UIControl::performPrimaryAction() {
    // Explicit programmatic dispatch, like sendActions. Input paths check
    // enabled/interaction gates before calling it.
    if (canBecomeFocused()) if (auto w=window()) w->focusSystem()->requestFocusUpdate(shared_from_base<UIFocusEnvironment>());
    dispatch(UIControlEvent::primaryActionTriggered,nullptr);
}
void UIControl::pressesBegan(std::set<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    if (isEnabled() && isActionDispatchAllowed() && std::any_of(presses.begin(),presses.end(),[](const auto& p){return p && !p->isHandled() && p->type()==UIPressType::select && !p->isRepeat();})) { _selectPressActive=true; setHighlighted(true); }
    UIView::pressesBegan(std::move(presses),std::move(event));
}
void UIControl::pressesEnded(std::set<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    auto unhandled=presses;
    for (const auto& press:presses) {
        if (!press || press->isHandled()) { unhandled.erase(press); continue; }
        if (performRegisteredAction(press)) { if (!press->isRepeat()) setHighlighted(false); unhandled.erase(press); continue; }
        if (press->type()!=UIPressType::select) continue;
        bool repeatable=primaryAction && primaryAction->repeatBehavior()==UIMenuElementRepeatBehavior::repeatable;
        for (const auto& h:_handlers) if (h.action && contains(h.events,UIControlEvent::primaryActionTriggered) && h.action->repeatBehavior()==UIMenuElementRepeatBehavior::repeatable) repeatable=true;
        const bool perform=isEnabled() && isActionDispatchAllowed() && isHighlighted() && contains(allControlEvents(),UIControlEvent::primaryActionTriggered) && (!press->isRepeat() || repeatable);
        if (!press->isRepeat()) { _selectPressActive=false; setHighlighted(false); }
        if (perform) { press->_isHandled=true; unhandled.erase(press); performPrimaryAction(); }
    }
    if (!unhandled.empty()) UIView::pressesEnded(std::move(unhandled),std::move(event));
}
void UIControl::pressesCancelled(std::set<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    if (std::any_of(presses.begin(),presses.end(),[](const auto& p){return p && p->type()==UIPressType::select;})) { _selectPressActive=false; setHighlighted(false); }
    UIView::pressesCancelled(std::move(presses),std::move(event));
}
void UIControl::didUpdateFocusIn(UIFocusUpdateContext context, UIFocusAnimationCoordinator* coordinator) {
    UIView::didUpdateFocusIn(context,coordinator); controlStateDidChange();
    const auto weak=weak_from_base<UIControl>();
    if (context.nextFocusedItem().lock()==shared_from_this()) {
        if (context.previouslyFocusedItem().lock()==context.nextFocusedItem().lock()) focusFailedToMove(context.focusHeading());
        else coordinator->addCoordinatedAnimations([weak]{if(auto self=weak.lock())self->willGainFocus();});
    } else coordinator->addCoordinatedAnimations([weak]{if(auto self=weak.lock()){self->setHighlighted(false);self->willLoseFocus();}});
}
void UIControl::willGainFocus() {}
void UIControl::willLoseFocus() {}
void UIControl::focusFailedToMove(UIFocusHeading) {}
void UIControl::willChangeFocusHighlight(bool) {}
void UIControl::willChangeHighlight(bool) {}
std::shared_ptr<UIView> UIControl::hitTest(NXPoint p,UIEvent* event) { return isEnabled()?UIView::hitTest(p,event):nullptr; }
void UIControl::setBaseScaleMultiplier(NXFloat value) {
    if (!std::isfinite(value) || value<=0) throw std::invalid_argument("Control scale must be positive and finite");
    if (_baseScaleMultiplier==value) return;
    _baseScaleMultiplier=value; baseScaleMultiplierDidChange();
}
bool UIControl::applyXMLAttribute(const std::string& name,const std::string& value) {
    if (name=="enabled") { if(auto v=valueToBool(value)){setEnabled(*v);return true;} return false; }
    if (name=="selected") { if(auto v=valueToBool(value)){setSelected(*v);return true;} return false; }
    if (name=="highlighted") { if(auto v=valueToBool(value)){setHighlighted(*v);return true;} return false; }
    if (name=="contentVerticalAlignment") {
        const std::vector<std::string> names={"center","top","bottom","fill"};
        const auto it=std::find(names.begin(),names.end(),value); if(it==names.end())return false;
        setContentVerticalAlignment(UIControlContentVerticalAlignment(it-names.begin()));return true;
    }
    if (name=="contentHorizontalAlignment") {
        const std::vector<std::string> names={"center","left","right","fill","leading","trailing"};
        const auto it=std::find(names.begin(),names.end(),value); if(it==names.end())return false;
        setContentHorizontalAlignment(UIControlContentHorizontalAlignment(it-names.begin()));return true;
    }
    return UIView::applyXMLAttribute(name,value);
}
}
