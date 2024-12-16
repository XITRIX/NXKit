#include <UIView.h>
#include <UIViewController.h>
#include <UIWindow.h>
#include <DispatchQueue.h>
#include <CASpringAnimationPrototype.h>

using namespace NXKit;

UIView::UIView(NXRect frame, std::shared_ptr<CALayer> layer) {
    _layer = layer;
    _layer->setAnchorPoint(NXPoint::zero);
    _layer->delegate = weak_from_this();
    setFrame(frame);
}

void UIView::setFrame(NXRect frame) {
    if (this->frame().size != frame.size) {
        setNeedsLayout();
    }
    _layer->setFrame(frame);
//    setNeedsUpdateSafeAreaInsets();
}

void UIView::setBounds(NXRect bounds) {
    if (this->bounds().size != bounds.size) {
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

void UIView::setSuperview(std::shared_ptr<UIView> superview) {
    _superview = superview;
}

void UIView::insertSubviewAt(std::shared_ptr<UIView> view, int index) {
    // TODO: Need to implement
}

void UIView::insertSubviewBelow(std::shared_ptr<UIView> view, std::shared_ptr<UIView> belowSubview) {
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

    if (visibleLayer->_needsDisplay) {
        visibleLayer->display();
        visibleLayer->_needsDisplay = false;
    }

    if (_needsDisplay) {
        draw();
        _needsDisplay = false;
    }

//    updateSafeAreaInsetsIfNeeded();
//    updateLayoutMarginIfNeeded();
    layoutIfNeeded();

    for (auto& subview: _subviews) {
        subview->drawAndLayoutTreeIfNeeded();
    }
}

void UIView::setMask(std::shared_ptr<UIView> mask) {
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

// MARK: - Animations
std::set<std::shared_ptr<CALayer>> UIView::layersWithAnimations;
std::shared_ptr<CABasicAnimationPrototype> UIView::currentAnimationPrototype;

bool UIView::anyCurrentlyRunningAnimationsAllowUserInteraction() {
    if (layer()->animations.empty()) return true;

    for (auto& animation: layer()->animations) {
        auto animationGroup = animation.second->animationGroup;
        if (animationGroup && (animationGroup->options & UIViewAnimationOptions::allowUserInteraction) == UIViewAnimationOptions::allowUserInteraction) {
            return true;
        }
    }

    return false;
}

void UIView::animate(double duration, double delay, UIViewAnimationOptions options, std::function<void()> animations, std::function<void(bool)> completion) {
    auto group = new_shared<UIViewAnimationGroup>(options, completion);
    currentAnimationPrototype = new_shared<CABasicAnimationPrototype>(duration, delay, group);

    animations();

    if (currentAnimationPrototype && currentAnimationPrototype->animationGroup->queuedAnimations == 0) {
        DispatchQueue::main()->async([completion]() { completion(true); });
    }

    currentAnimationPrototype = nullptr;
}


void UIView::animate(double duration, std::function<void()> animations, std::function<void(bool)> completion) {
    UIView::animate( duration, 0, UIViewAnimationOptions::none, animations, completion);
}

void UIView::animate(double duration, double delay, double damping, double initialSpringVelocity, UIViewAnimationOptions options, std::function<void()> animations, std::function<void(bool)> completion) {
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

    auto keyPath = event;
    auto beginFromCurrentState = (prototype->animationGroup->options & UIViewAnimationOptions::beginFromCurrentState) == UIViewAnimationOptions::beginFromCurrentState;

    auto state = beginFromCurrentState ? (_layer->presentationOrSelf()) : _layer;

    auto fromValue = state->value(keyPath);

    if (fromValue.has_value()) {
        return prototype->createAnimation(keyPath, fromValue.value());
    }

    return nullptr;
}

void UIView::display(std::shared_ptr<CALayer> layer) { }

// MARK: - Layout
void UIView::setNeedsLayout() {
    setNeedsDisplay();

    // Needs to recalculate Yoga from root
//    auto layoutRoot = this->layoutRoot();
//    if (layoutRoot) layoutRoot->_needsLayout = true;
    _needsLayout = true;
}

void UIView::layoutIfNeeded() {
    if (_needsLayout) {
        _needsLayout = false;
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
//    yoga->layoutIfNeeded();
//    updateSafeAreaInsets();

    if (!_parentController.expired()) {
        _parentController.lock()->viewDidLayoutSubviews();
    }
}

NXSize UIView::sizeThatFits(NXSize size) {
    // Apple's implementation returns current view's bounds().size
    // But in case we use Yoga's autolayout it will be better to replace it
    // with zero Size value, to allow Yoga to work properly
//    return NXSize();
    return bounds().size;
}

void UIView::sizeToFit() {
    auto bounds = this->bounds();
    bounds.size = sizeThatFits(bounds.size);
    setBounds(bounds);
}
