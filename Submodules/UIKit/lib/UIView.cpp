#include <UIView.h>
#include <UIViewController.h>
#include <UIWindow.h>
#include <DispatchQueue.h>
#include <CASpringAnimationPrototype.h>
#include <UIGestureRecognizer.h>

#include <utility>

using namespace NXKit;

UIView::UIView(NXRect frame, std::shared_ptr<CALayer> layer) {
    _yoga = new_shared<YGLayout>(shared_from_this());
//    _yoga->setEnabled(true);

    _layer = std::move(layer);
    _layer->setAnchorPoint(NXPoint::zero);
    _layer->delegate = weak_from_this();
    setFrame(frame);
}

std::shared_ptr<UIResponder> UIView::next() {
    if (!_parentController.expired()) return _parentController.lock();
    if (!_superview.expired()) return _superview.lock();
    return nullptr;
}

void UIView::setFrame(NXRect frame) {
    if (this->frame().size != frame.size) {
        setNeedsDisplay();
        setNeedsLayout();
    }
    _layer->setFrame(frame);
//    setNeedsUpdateSafeAreaInsets();
}

void UIView::setBounds(NXRect bounds) {
    if (this->bounds().size != bounds.size) {
        setNeedsDisplay();
        setNeedsLayout();
//        setNeedsUpdateSafeAreaInsets();
    }
    _layer->setBounds(bounds);
}

void UIView::setCenter(NXPoint position) {
    auto frame = this->frame();
    frame.setMidX(position.x);
    frame.setMidY(position.y);
    setFrame(frame);
}

NXPoint UIView::center() const {
    auto frame = this->frame();
    return { frame.midX(), frame.midY() };
}

void UIView::addGestureRecognizer(const std::shared_ptr<UIGestureRecognizer>& gestureRecognizer) {
    gestureRecognizer->_view = weak_from_this();
    _gestureRecognizers.push_back(gestureRecognizer);
}

void UIView::addSubview(std::shared_ptr<UIView> view) {
    bool needToNotifyViewController = false;
    if (!view->_parentController.expired()) {
        auto window = this->window();
        if (window) {
            needToNotifyViewController = true;
        }
    }

    setNeedsLayout();
    view->removeFromSuperview();

    if (needToNotifyViewController)
        view->_parentController.lock()->viewWillAppear(true);

    _layer->addSublayer(view->_layer);
    _subviews.push_back(view);
    view->setSuperview(this->shared_from_this());
//    view->setNeedsUpdateSafeAreaInsets();
}

std::shared_ptr<UIWindow> UIView::window() {
    if (!_superview.expired()) return _superview.lock()->window();
    return nullptr;
}

void UIView::setSuperview(const std::shared_ptr<UIView>& superview) {
    _superview = superview;
}

void UIView::insertSubviewAt(std::shared_ptr<UIView> view, int index) {
    // TODO: Need to implement
}

void UIView::insertSubviewBelow(const std::shared_ptr<UIView>& view, const std::shared_ptr<UIView>& belowSubview) {
    auto itr = std::find(subviews().cbegin(), subviews().cend(), belowSubview);
    if (itr == subviews().cend()) { return; }

    bool needToNotifyViewController = false;
    if (!view->_parentController.expired()) {
        auto window = this->window();
        if (window) {
            needToNotifyViewController = true;
        }
    }

    setNeedsLayout();
    view->removeFromSuperview();

    if (needToNotifyViewController)
        view->_parentController.lock()->viewWillAppear(true);

    _layer->insertSublayerBelow(view->_layer, belowSubview->layer());
    _subviews.insert(itr, view);
    view->setSuperview(this->shared_from_this());
//    view->setNeedsUpdateSafeAreaInsets();
}

void UIView::removeFromSuperview() {
    auto superview = this->_superview.lock();
    if (!superview) return;

    _layer->removeFromSuperlayer();

    // If it's mask - remove
    if (superview->_mask.get() == this) {
        superview->_mask = nullptr;
    }
    else {
        superview->_subviews.erase(std::remove(superview->_subviews.begin(), superview->_subviews.end(), shared_from_this()), superview->_subviews.end());
    }
    this->setSuperview(nullptr);
    superview->setNeedsLayout();
}

void UIView::drawAndLayoutTreeIfNeeded() {
    auto visibleLayer = layer()->presentationOrSelf();

    if (visibleLayer->isHidden() || visibleLayer->opacity() < 0.01f) { return; }

    UITraitCollection::setCurrent(traitCollection());

    if (_contentMode == UIViewContentMode::redraw) {
        if (visibleLayer->contents() && visibleLayer->contents()->size() != visibleLayer->bounds().size) {
            setNeedsDisplay();
        }
    }

    UITraitCollection::setCurrent(traitCollection());

    if (visibleLayer->_needsDisplay) {
        visibleLayer->display();
        visibleLayer->_needsDisplay = false;
    }

    UITraitCollection::setCurrent(traitCollection());

    if (_needsDisplay) {
        draw();
        _needsDisplay = false;
    }

    UITraitCollection::setCurrent(traitCollection());

//    updateSafeAreaInsetsIfNeeded();
//    updateLayoutMarginIfNeeded();
    layoutIfNeeded();
    _yoga->layoutIfNeeded();

    for (auto& subview: _subviews) {
        subview->drawAndLayoutTreeIfNeeded();
    }
}

void UIView::setMask(const std::shared_ptr<UIView>& mask) {
    if (_mask == mask) { return; }
    if (_mask) { _mask->removeFromSuperview(); }

    _mask = mask;
    if (mask) {
        _layer->setMask(mask->_layer);
        mask->setSuperview(shared_from_this());
    } else {
        _layer->setMask(nullptr);
    }
}

void UIView::setContentMode(UIViewContentMode mode) {
    if (_contentMode == mode) return;
    _contentMode = mode;
    switch (mode) {
        case UIViewContentMode::scaleToFill:
            _layer->setContentsGravity(CALayerContentsGravity::resize);
            break;
        case UIViewContentMode::scaleAspectFit:
            _layer->setContentsGravity(CALayerContentsGravity::resizeAspect);
            break;
        case UIViewContentMode::scaleAspectFill:
            _layer->setContentsGravity(CALayerContentsGravity::resizeAspectFill);
            break;
        case UIViewContentMode::redraw:
            _layer->setContentsGravity(CALayerContentsGravity::resize);
            break;
        case UIViewContentMode::center:
            _layer->setContentsGravity(CALayerContentsGravity::center);
            break;
        case UIViewContentMode::top:
            _layer->setContentsGravity(CALayerContentsGravity::top);
            break;
        case UIViewContentMode::bottom:
            _layer->setContentsGravity(CALayerContentsGravity::bottom);
            break;
        case UIViewContentMode::left:
            _layer->setContentsGravity(CALayerContentsGravity::left);
            break;
        case UIViewContentMode::right:
            _layer->setContentsGravity(CALayerContentsGravity::right);
            break;
        case UIViewContentMode::topLeft:
            _layer->setContentsGravity(CALayerContentsGravity::topLeft);
            break;
        case UIViewContentMode::topRight:
            _layer->setContentsGravity(CALayerContentsGravity::topRight);
            break;
        case UIViewContentMode::bottomLeft:
            _layer->setContentsGravity(CALayerContentsGravity::bottomLeft);
            break;
        case UIViewContentMode::bottomRight:
            _layer->setContentsGravity(CALayerContentsGravity::bottomRight);
            break;
    }
}

// MARK: - Touch
NXPoint UIView::convertFromView(NXPoint point, const std::shared_ptr<UIView>& fromView) {
    if (!fromView) return point;
    return fromView->convertToView(point, shared_from_this());
}

NXPoint UIView::convertToView(NXPoint point, const std::shared_ptr<UIView>& toView) const {
    NXPoint selfAbsoluteOrigin;
    NXPoint otherAbsoluteOrigin;

    const UIView* current = this;
    while (current) {
        if (current == toView.get()) {
            return point + selfAbsoluteOrigin;
        }
        selfAbsoluteOrigin += current->frame().origin;
        selfAbsoluteOrigin -= current->bounds().origin;
        auto superview = current->_superview.lock();
        if (!superview) break;

        current = superview.get();
    }

    current = toView.get();
    while (current) {
        otherAbsoluteOrigin += current->frame().origin;
        otherAbsoluteOrigin -= current->bounds().origin;
        auto superview = current->_superview.lock();
        if (!superview) break;

        current = superview.get();
    }

    NXPoint originDifference = otherAbsoluteOrigin - selfAbsoluteOrigin;
    return point - originDifference;
}

std::shared_ptr<UIView> UIView::hitTest(NXPoint point, UIEvent* withEvent) {
    if (isHidden() || !_isUserInteractionEnabled || alpha() < 0.01 || !anyCurrentlyRunningAnimationsAllowUserInteraction())
        return nullptr;

    if (!this->point(point, withEvent))
        return nullptr;

    auto subviews = _subviews;
    for (int i = (int) subviews.size() - 1; i >= 0; i--) {
        NXPoint convertedPoint = shared_from_this()->convertToView(point, subviews[i]);
        std::shared_ptr<UIView> test = subviews[i]->hitTest(convertedPoint, withEvent);
        if (test) return test;
    }

    return shared_from_this();
}

bool UIView::point(NXPoint insidePoint, UIEvent* withEvent) {
    return bounds().contains(insidePoint);
}

// MARK: - Animations
std::set<std::shared_ptr<CALayer>> UIView::layersWithAnimations;
std::shared_ptr<CABasicAnimationPrototype> UIView::currentAnimationPrototype;

bool UIView::anyCurrentlyRunningAnimationsAllowUserInteraction() const {
    if (layer()->animations.empty()) return true;

    for (auto& animation: layer()->animations) {
        auto animationGroup = animation.second->animationGroup;
        if (animationGroup && (animationGroup->options & UIViewAnimationOptions::allowUserInteraction) == UIViewAnimationOptions::allowUserInteraction) {
            return true;
        }
    }

    return false;
}

void UIView::animate(double duration, double delay, UIViewAnimationOptions options, const std::function<void()>& animations, std::function<void(bool)> completion) {
    auto group = new_shared<UIViewAnimationGroup>(options, completion);
    currentAnimationPrototype = new_shared<CABasicAnimationPrototype>(duration, delay, group);

    animations();

    if (currentAnimationPrototype && currentAnimationPrototype->animationGroup->queuedAnimations == 0) {
        DispatchQueue::main()->async([completion]() { completion(true); });
    }

    currentAnimationPrototype = nullptr;
}


void UIView::animate(double duration, const std::function<void()>& animations, std::function<void(bool)> completion) {
    UIView::animate( duration, 0, UIViewAnimationOptions::none, animations, std::move(completion));
}

void UIView::animate(double duration, double delay, double damping, double initialSpringVelocity, UIViewAnimationOptions options, const std::function<void()>& animations, std::function<void(bool)> completion) {
    auto group = new_shared<UIViewAnimationGroup>(options, completion);
    currentAnimationPrototype = new_shared<CASpringAnimationPrototype>( duration, delay, damping, initialSpringVelocity, group);

    animations();

    if (currentAnimationPrototype && currentAnimationPrototype->animationGroup->queuedAnimations == 0) {
        completion(true);
    }
    currentAnimationPrototype = nullptr;
}

void UIView::animateIfNeeded(Timer currentTime) {
    auto layersWithAnimationsCopy = layersWithAnimations;
    for (auto& layer: layersWithAnimationsCopy) {
        layer->animateAt(currentTime);
    }
}

void UIView::completePendingAnimations() {
    for (auto& layer: layersWithAnimations) {
        timeval now;
        gettimeofday(&now, nullptr);
        // FIXME: incorrect logic
        layer->animateAt(Timer(timevalInMilliseconds(now) + 1000000000));
//        $0.animate(at: Timer(startingAt: NSDate.distantFuture.timeIntervalSinceNow));
    }
}

std::shared_ptr<CABasicAnimation> UIView::actionForKey(std::string event) {
    auto prototype = UIView::currentAnimationPrototype;
    if (!prototype) { return nullptr; }

    const auto& keyPath = event;
    auto beginFromCurrentState = (prototype->animationGroup->options & UIViewAnimationOptions::beginFromCurrentState) == UIViewAnimationOptions::beginFromCurrentState;

    auto state = beginFromCurrentState ? (_layer->presentationOrSelf()) : _layer;

    auto fromValue = state->value(keyPath);

    if (fromValue.has_value()) {
        return prototype->createAnimation(keyPath, fromValue.value());
    }

    return nullptr;
}

void UIView::display(std::shared_ptr<CALayer> layer) { }

void UIView::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) {
    UITraitEnvironment::traitCollectionDidChange(previousTraitCollection);
    for (const auto& subview : _subviews) {
        // If subview has controller, it's controller will update related traitCollection
        if (!subview->_parentController.expired()) continue;

        subview->_traitCollection = _traitCollection;
        subview->traitCollectionDidChange(previousTraitCollection);
    }
}

// MARK: - Layout
std::shared_ptr<UIView> UIView::layoutRoot() {
    auto view = shared_from_this();
    while (true) {
        if (view->_yoga->isRoot() || view->superview().expired()) return view;
        view = view->superview().lock();
    }
//    while (view && !view->_yoga->isRoot()) {
//        view = view->superview().lock();
//    }
//    return view;
}

void UIView::setNeedsLayout() {
//    setNeedsDisplay();

    // Needs to recalculate Yoga from root
    auto layoutRoot = this->layoutRoot();
    if (layoutRoot) layoutRoot->_needsLayout = true;
    _needsLayout = true;
}

void UIView::layoutIfNeeded() {
    if (_needsLayout) {
        _needsLayout = false;

        layoutRoot()->layoutIfNeeded();

        for (const auto &view : subviews()) {
            view->setNeedsLayout();
        }
        layoutSubviews();
    }
}

void UIView::layoutSubviews() {
    _needsLayout = false;
    if (!_parentController.expired()) {
        _parentController.lock()->viewWillLayoutSubviews();
    }

//    updateEdgeInsets();
    _yoga->layoutIfNeeded();
//    yoga->applyLayoutPreservingOrigin(true);
//    updateSafeAreaInsets();

    if (!_parentController.expired()) {
        _parentController.lock()->viewDidLayoutSubviews();
    }
}

NXSize UIView::sizeThatFits(NXSize size) {
    // Apple's implementation returns current view's bounds().size
    // But in case we use Yoga's autolayout it will be better to replace it
    // with zero Size value, to allow Yoga to work properly
    return NXSize();
//    return bounds().size;
}

void UIView::sizeToFit() {
    NXRect bounds;
    if (!superview().expired()) {
        bounds = this->superview().lock()->bounds();
    } else {
        bounds = this->bounds();
    }
    
    bounds.size = sizeThatFits(bounds.size);
    setBounds(bounds);
}

// MARK: - Yoga layout
void UIView::configureLayout(const std::function<void(std::shared_ptr<YGLayout>)>& block) {
    _yoga->setEnabled(true);
    block(_yoga);
}
