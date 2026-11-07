#include <UIWindow.h>
#include <UIApplication.h>
#include <UIPresentationController.h>
#include <UIViewController.h>
#include <UITouch.h>
#include <UIPress.h>
#include <UIPressesEvent.h>
#include <UIScrollView.h>
#include <SkiaCtx.h>

#include <algorithm>
#include <deque>

using namespace NXKit;

struct UIWindow::ResponderTouchDelivery {
    struct State {
        std::shared_ptr<UITouch> touch;
        UITouchPhase phase;
        Timer timestamp;
        NXPoint location, previousLocation;
    };
    struct Sample {
        std::shared_ptr<UIEvent> event;
        std::vector<State> states;
        UITouchPhase phase;
    };
    std::shared_ptr<UITouch> touch;
    std::weak_ptr<UIView> target;
    std::deque<Sample> pending;
    Sample latest;
    bool beganDelivered = false, terminalDelivered = false;
    bool suppressed = false, invalidated = false, inputFinished = false;
};

void UIWindow::queueResponderTouch(const std::shared_ptr<UITouch>& touch,
                                  const std::shared_ptr<UIEvent>& event,
                                  const std::shared_ptr<UIView>& target) {
    auto found = std::find_if(_responderTouchDeliveries.begin(), _responderTouchDeliveries.end(),
        [&](const auto& delivery) { return delivery->touch == touch; });
    if (found == _responderTouchDeliveries.end()) {
        if (touch->phase() != UITouchPhase::began) return;
        auto delivery = std::make_shared<ResponderTouchDelivery>();
        delivery->touch = touch; delivery->target = target;
        _responderTouchDeliveries.push_back(delivery);
        found = std::prev(_responderTouchDeliveries.end());
    }
    auto& delivery = **found;
    ResponderTouchDelivery::Sample sample;
    sample.phase = touch->phase();
    // A UIEvent may be reused by the input source. Freeze its membership and
    // each contact's original state without replacing the UITouch identities.
    sample.event = new_shared<UIEvent>();
    sample.event->_timestamp = event->timestamp();
    sample.event->_allTouches = event->allTouches();
    sample.event->_allTouches.insert(touch);
    for (const auto& contact: sample.event->_allTouches)
        sample.states.push_back({contact, contact->_phase, contact->_timestamp,
                                 contact->_absoluteLocation, contact->_previousAbsoluteLocation});
    delivery.latest = sample;
    delivery.inputFinished = sample.phase == UITouchPhase::ended || sample.phase == UITouchPhase::cancelled;
    if (!delivery.suppressed) delivery.pending.push_back(std::move(sample));
}

void UIWindow::invalidateResponderTouchesForView(const UIView* view) {
    for (const auto& delivery: _responderTouchDeliveries) {
        auto target = delivery->target.lock();
        while (target) {
            if (target.get() == view) { delivery->invalidated = true; break; }
            target = target->superview().lock();
        }
    }
    flushResponderTouches();
}

bool UIWindow::hasDeliveredResponderTouch(const std::shared_ptr<UITouch>& touch) const {
    return std::any_of(_responderTouchDeliveries.begin(), _responderTouchDeliveries.end(),
        [&](const auto& delivery) {
            return delivery->touch == touch && delivery->beganDelivered
                && !delivery->terminalDelivered && !delivery->suppressed;
        });
}

void UIWindow::flushResponderTouches() {
    if (_flushingResponderTouches) { _responderTouchFlushRequested = true; return; }
    _flushingResponderTouches = true;
    struct FlushScope { bool& flag; ~FlushScope() { flag = false; } } scope{_flushingResponderTouches};
    do {
        _responderTouchFlushRequested = false;
        const auto deliveries = _responderTouchDeliveries;
        for (const auto& delivery: deliveries) {
            const auto target = delivery->target.lock();
            const auto dispatch = [&](ResponderTouchDelivery::Sample sample, UITouchPhase phase) {
                if (!target) return;
                std::vector<ResponderTouchDelivery::State> restore;
                for (const auto& state: sample.states) {
                    auto& contact = *state.touch;
                    restore.push_back({state.touch, contact._phase, contact._timestamp,
                                       contact._absoluteLocation, contact._previousAbsoluteLocation});
                    contact._phase = state.touch == delivery->touch ? phase : state.phase;
                    contact._timestamp = state.timestamp;
                    contact._absoluteLocation = state.location;
                    contact._previousAbsoluteLocation = state.previousLocation;
                }
                const auto restoreState = [&] {
                    for (const auto& state: restore) {
                        state.touch->_phase = state.phase; state.touch->_timestamp = state.timestamp;
                        state.touch->_absoluteLocation = state.location;
                        state.touch->_previousAbsoluteLocation = state.previousLocation;
                    }
                };
                try {
                    const std::set<std::shared_ptr<UITouch>> changed{delivery->touch};
                    if (phase == UITouchPhase::began) {
                        std::vector<std::shared_ptr<UIScrollView>> ancestors;
                        auto ancestor = target->superview().lock();
                        while (ancestor) {
                            if (auto scroll = std::dynamic_pointer_cast<UIScrollView>(ancestor)) ancestors.push_back(scroll);
                            ancestor = ancestor->superview().lock();
                        }
                        for (const auto& scroll: ancestors) {
                            if (!scroll->touchesShouldBegin(changed,sample.event,target)) {
                                delivery->suppressed = true;
                                delivery->pending.clear();
                                restoreState();
                                return;
                            }
                            if (delivery->invalidated || target->window().get() != this
                                || !target->isActionDispatchAllowed()) {
                                delivery->suppressed = true;
                                delivery->pending.clear();
                                restoreState();
                                return;
                            }
                        }
                        delivery->beganDelivered = true;
                    }
                    switch (phase) {
                        case UITouchPhase::began: target->touchesBegan(changed, sample.event); break;
                        case UITouchPhase::moved: target->touchesMoved(changed, sample.event); break;
                        case UITouchPhase::ended: target->touchesEnded(changed, sample.event); break;
                        case UITouchPhase::cancelled: target->touchesCancelled(changed, sample.event); break;
                    }
                } catch (...) { restoreState(); throw; }
                restoreState();
            };
            while (!delivery->suppressed && !delivery->terminalDelivered) {
                const bool unavailable = delivery->invalidated || !target
                    || target->window().get() != this || !target->isActionDispatchAllowed();
                if (unavailable || delivery->touch->_hasBeenCancelledByAGestureRecognizer
                    || delivery->latest.phase == UITouchPhase::cancelled) {
                    delivery->suppressed = true;
                    delivery->pending.clear();
                    if (delivery->beganDelivered) {
                        delivery->terminalDelivered = true;
                        dispatch(delivery->latest, UITouchPhase::cancelled);
                    }
                    break;
                }
                // Every recognizer sees the raw input before normal responder
                // delivery resumes. Cancellation is allowed before its action.
                if (_touchDispatchDepth || delivery->pending.empty()) break;
                const auto phase = delivery->pending.front().phase;
                bool delayed = false;
                if (phase == UITouchPhase::began || phase == UITouchPhase::ended) {
                    for (const auto& weak: delivery->touch->_gestureRecognizers) {
                        const auto recognizer = weak.lock();
                        if (!recognizer || !recognizer->_isEnabled || recognizer->_attemptCancelled
                            || recognizer->_state != UIGestureRecognizerState::possible) continue;
                        if (std::find(recognizer->_allTouches.begin(), recognizer->_allTouches.end(), delivery->touch)
                            == recognizer->_allTouches.end()) continue;
                        delayed |= phase == UITouchPhase::began ? recognizer->_delaysTouchesBegan : recognizer->_delaysTouchesEnded;
                    }
                }
                if (delayed) break;
                auto sample = std::move(delivery->pending.front());
                delivery->pending.pop_front();
                if (phase == UITouchPhase::ended) delivery->terminalDelivered = true;
                dispatch(std::move(sample), phase);
            }
        }
        std::erase_if(_responderTouchDeliveries, [](const auto& delivery) {
            return delivery->inputFinished && (delivery->suppressed || delivery->terminalDelivered);
        });
    } while (_responderTouchFlushRequested);
}

std::vector<std::weak_ptr<UIGestureRecognizer>> getRecognizerHierachyFromView(std::shared_ptr<UIView> view) {
    std::vector<std::weak_ptr<UIGestureRecognizer>> recognizers;
    while (view) {
        for (const auto& recognizer : *view->gestureRecognizers())
            recognizers.push_back(recognizer);
        view = view->superview().lock();
    }

    return recognizers;
}

UIWindow::UIWindow() {
    _focusSystem = new_shared<UIFocusSystem>();
    _focusSystem->_rootWindow = weak_from_base<UIWindow>();

    setBackgroundColor(UIColor::systemBackground);
    // UIView supplies the default tint. Keep it inherited so component kits
    // can distinguish an application's explicit tint from that default.

//    yoga->setEnabled(false);
}

UIWindow::~UIWindow() {
    _presentedViewControllers.clear();
}

std::shared_ptr<UIWindow> UIWindow::window() {
    return shared_from_base<UIWindow>();
}

void UIWindow::setRootViewController(std::shared_ptr<UIViewController> rootViewController) {
    if (_rootViewController) {
        _rootViewController->view()->removeFromSuperview();
    }

    _rootViewController = std::move(rootViewController);
    if (_rootViewController && _traitCollection) {
        const auto previous = _rootViewController->_traitCollection;
        _rootViewController->_traitCollection = _traitCollection;
        _rootViewController->traitCollectionDidChange(previous);
    }
}

void UIWindow::makeKeyAndVisible() {
//    self.safeAreaInsets = UIWindow.getSafeAreaInsets()
    auto window = std::static_pointer_cast<UIWindow>(shared_from_this());
//    window->setBounds(UIRenderer::main()->bounds());
    UIApplication::shared->keyWindow = window;
    if (const auto context = SkiaCtx::main()) setFrame({NXPoint::zero, context->getSize()});
    // Supply traits before loadView/viewDidLoad and initial appearance callbacks.
    updateTraitsFromPlatform();

    auto viewController = _rootViewController;
    if (viewController) {
        viewController->loadViewIfNeeded();
        viewController->view()->setFrame(this->bounds());
        viewController->viewWillAppear(false);
        addSubview(viewController->view());
        layoutIfNeeded();
        viewController->viewDidAppear(false);
        updateFocus();
    }
}

void UIWindow::updateFocus() {
    _focusSystem->updateFocus();
}

std::shared_ptr<UIView> UIWindow::focusRootView() {
    if (!_presentedViewControllers.empty()) {
        return _presentedViewControllers.back()->view();
    }
    if (_rootViewController) {
        return _rootViewController->view();
    }
    return shared_from_base<UIWindow>();
}

void UIWindow::sendEvent(const std::shared_ptr<UIEvent>& event) {
    focusSystem()->sendEvent(event);

    if (auto pevent = std::dynamic_pointer_cast<UIPressesEvent>(event)) {
        sendPressEvent(pevent);
    } else {
        sendTouchEvent(event);
    }
}

void UIWindow::sendTouchEvent(std::shared_ptr<UIEvent> event) {
    ++_touchDispatchDepth;
    const auto finishDispatch = [&] { --_touchDispatchDepth; flushResponderTouches(); };
    try {
        for (const auto& touch: event->allTouches()) {
            _inputType = UIWindowInputType::touch;
            auto hitView = touch->view().lock();
            if (!hitView && touch->phase() == UITouchPhase::began)
                hitView = hitTest(touch->locationIn(nullptr), nullptr);
            if (!hitView && touch->phase() == UITouchPhase::began) continue;
            touch->_window = shared_from_base<UIWindow>();
            if (touch->phase() == UITouchPhase::began) {
                const auto elapsed = touch->timestamp() - _lastTapTime;
                if (touch->_tapCount == 1 && _activeTouchIds.empty() && _lastTapCount
                    && _lastTapView.lock() == hitView && elapsed >= 0 && elapsed <= 300
                    && (touch->absoluteLocation()-_lastTapPoint).magnitude() <= 22)
                    touch->_tapCount = std::min(32u,_lastTapCount+1);
                _lastTapCount = 0;
                _tapContactIsolated = _activeTouchIds.empty();
                _activeTouchIds.insert(touch->touchId());
                touch->_view = hitView;
                touch->_gestureRecognizers = getRecognizerHierachyFromView(hitView);
            } else if (touch->phase() == UITouchPhase::ended) {
                const bool isolated = _tapContactIsolated && _activeTouchIds.size() == 1;
                _activeTouchIds.erase(touch->touchId());
                if (hitView && isolated && !touch->hasBeenCancelledByAGestureRecognizer()
                    && (touch->absoluteLocation()-touch->_initialLocation).magnitude() <= 22) {
                    _lastTapView=hitView; _lastTapPoint=touch->absoluteLocation();
                    _lastTapTime=touch->timestamp(); _lastTapCount=touch->tapCount();
                }
            } else if (touch->phase() == UITouchPhase::cancelled) {
                _activeTouchIds.erase(touch->touchId()); _lastTapCount = 0;
            }
            queueResponderTouch(touch,event,hitView);
            UIGestureRecognizer::beginEventDispatch();
            try {
                touch->runTouchActionOnRecognizerHierachy([&](const auto& recognizer) {
                    switch (touch->phase()) {
                        case UITouchPhase::began: recognizer->_touchesBegan({touch},event); break;
                        case UITouchPhase::moved: recognizer->_touchesMoved({touch},event); break;
                        case UITouchPhase::ended: recognizer->_touchesEnded({touch},event); break;
                        case UITouchPhase::cancelled: recognizer->_touchesCancelled({touch},event); break;
                    }
                });
            } catch (...) { UIGestureRecognizer::endEventDispatch(); throw; }
            UIGestureRecognizer::endEventDispatch();
        }
    } catch (...) { finishDispatch(); throw; }
    finishDispatch();
}

void UIWindow::sendPressEvent(const std::shared_ptr<UIPressesEvent>& event) {
    _inputType = UIWindowInputType::focus;
//    _focusSystem->setActive(true);

    for (auto& press: event->allPresses()) {
        if (press->responder().expired()) continue;

        const auto responder = press->responder().lock();
        if (press->phase() == UIPressPhase::began && !press->isRepeat()) {
            auto current = responder;
            while (current) {
                if (const auto view = std::dynamic_pointer_cast<UIView>(current)) {
                    press->_gestureRecognizers = getRecognizerHierachyFromView(view);
                    break;
                }
                current = current->next();
            }
        }
        const bool wasCancelled = press->hasBeenCancelledByAGestureRecognizer();
        if (!press->isRepeat()) {
            UIGestureRecognizer::beginEventDispatch();
            try {
                // Unlike responder delivery, recognizers all see the same
                // event even after a competing recognizer has cancelled it.
                const auto recognizers = press->_gestureRecognizers;
                for (const auto& weak : recognizers) {
                    const auto recognizer = weak.lock();
                    if (!recognizer || !recognizer->isEnabled()) continue;
                    switch (press->phase()) {
                        case UIPressPhase::began: recognizer->_pressesBegan({press}, event); break;
                        case UIPressPhase::changed:
                        case UIPressPhase::stationary: recognizer->_pressesChanged({press}, event); break;
                        case UIPressPhase::ended: recognizer->_pressesEnded({press}, event); break;
                        case UIPressPhase::cancelled: recognizer->_pressesCancelled({press}, event); break;
                    }
                }
            } catch (...) { UIGestureRecognizer::endEventDispatch(); throw; }
            UIGestureRecognizer::endEventDispatch();
        }
        if (press->hasBeenCancelledByAGestureRecognizer()) {
            if (!wasCancelled) responder->pressesCancelled({press}, event);
            continue;
        }

        switch (press->phase()) {
            case UIPressPhase::began: {
                press->responder().lock()->pressesBegan({ press }, event);
                break;
            }
            case UIPressPhase::ended: {
                press->responder().lock()->pressesEnded({ press }, event);
                break;
            }
            case UIPressPhase::changed:
            case UIPressPhase::stationary: {
                responder->pressesChanged({press}, event);
                break;
            }
            case UIPressPhase::cancelled: {
                press->responder().lock()->pressesCancelled({ press }, event);
                break;
            }
            default:
                break;
        }
    }
}

void UIWindow::updateTraitsFromPlatform() {
    const auto context = SkiaCtx::main();
    if (!context) return;
    const auto [horizontal, vertical] = context->getSizeClasses();
    const auto style = context->getThemeMode();
    const auto glassTintAmount = UIApplication::shared
        ? UIApplication::shared->glassTintAmount() : UITraitCollection::defaultGlassTintAmount;
    const auto scale = context->getScaleFactor();
    if (_traitCollection && _traitCollection->horizontalSizeClass() == horizontal
        && _traitCollection->verticalSizeClass() == vertical
        && _traitCollection->userInterfaceStyle() == style
        && _traitCollection->glassTintAmount() == glassTintAmount
        && _traitCollection->displayScale() == scale) return;

    const auto previous = _traitCollection;
    auto next = previous ? previous->copy() : std::make_shared<UITraitCollection>();
    next->_horizontalSizeClass = horizontal;
    next->_verticalSizeClass = vertical;
    next->_userInterfaceStyle = style;
    next->_glassTintAmount = glassTintAmount;
    next->_displayScale = scale;
    _traitCollection = next;
    // Match the key window's current environment before delivering callbacks;
    // the old collection remains an immutable snapshot for client comparisons.
    const auto oldCurrent = UITraitCollection::current();
    UITraitCollection::setCurrent(next);
    traitCollectionDidChange(previous);
    UITraitCollection::setCurrent(oldCurrent);
    setNeedsLayout();
    layer()->setNeedsDisplay();
}

void UIWindow::layoutSubviews() {
    if (const auto context = SkiaCtx::main()) setSafeAreaInsets(context->deviceSafeAreaInsets());

    if (_rootViewController) {
        _rootViewController->view()->setFrame(this->bounds());
    }

    for (const auto& viewController : _presentedViewControllers) {
        const auto presentationController = viewController->presentationController();
        if (!presentationController) {
            viewController->view()->setFrame(this->bounds());
            continue;
        }

        presentationController->containerViewWillLayoutSubviews();
        viewController->view()->setFrame(
            presentationController->frameOfPresentedViewInContainerView()
        );
        presentationController->containerViewDidLayoutSubviews();
    }

    UIView::layoutSubviews();
}

void UIWindow::addPresentedViewController(const std::shared_ptr<UIViewController>& controller) {
    if (std::find(
            _presentedViewControllers.begin(),
            _presentedViewControllers.end(),
            controller
        ) == _presentedViewControllers.end()) {
        _presentedViewControllers.push_back(controller);
    }
}

void UIWindow::removePresentedViewController(const std::shared_ptr<UIViewController>& controller) {
    _presentedViewControllers.erase(std::remove(_presentedViewControllers.begin(), _presentedViewControllers.end(), controller), _presentedViewControllers.end());
}

void UIWindow::pressesBegan(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    UIView::pressesBegan(pressees, event);
}

void UIWindow::pressesChanged(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    UIView::pressesChanged(pressees, event);
//    focusSystem()->sendEvent(event);
}

void UIWindow::pressesEnded(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    UIView::pressesEnded(pressees, event);
}

void UIWindow::pressesCancelled(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    UIView::pressesCancelled(pressees, event);
//    focusSystem()->sendEvent(event);
}

void UIWindow::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) {
    const auto root = _rootViewController;
    // Ordinary window-owned views (including presentation chrome) inherit
    // here; controller-owned roots update through their controllers below.
    UIView::traitCollectionDidChange(previousTraitCollection);
    if (root && _rootViewController == root) {
        const auto previous = root->_traitCollection;
        root->_traitCollection = _traitCollection;
        root->traitCollectionDidChange(previous);
    }
}
