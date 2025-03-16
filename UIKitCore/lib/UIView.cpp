#include <UIView.h>
#include <UIViewController.h>
#include <UIWindow.h>
#include <DispatchQueue.h>
#include <CASpringAnimationPrototype.h>
#include <UIGestureRecognizer.h>
#include <UINib.h>
#include <tools/IBTools.h>

#include <utility>

using namespace NXKit;

std::shared_ptr<UIView> UIView::instantiateFromXib(tinyxml2::XMLElement* element, std::map<std::string, std::shared_ptr<UIView>>* idStorage) {
    auto name = element->Name();
    auto view = UINib::xibViewsRegister[name]();
    view->setAutolayoutEnabled(true);
    view->applyXMLAttributes(element, idStorage);

    for (tinyxml2::XMLElement* child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
        auto subview = instantiateFromXib(child, idStorage);
        view->addSubview(subview);
    }

    return view;
}

void UIView::applyXMLAttributes(tinyxml2::XMLElement* element, std::map<std::string, std::shared_ptr<UIView>>* parsingIdStorage) {
    if (!element)
        return;

    for (const tinyxml2::XMLAttribute* attribute = element->FirstAttribute(); attribute != nullptr; attribute = attribute->Next())
    {
        std::string name  = attribute->Name();
        std::string value = std::string(attribute->Value());

        if (name == "id") {
            tag = value;
            if (!parsingIdStorage) continue;
            parsingIdStorage->insert(std::pair<std::string, std::shared_ptr<UIView>>(value, shared_from_this()));
            continue;
        }

        if (!this->applyXMLAttribute(name, value)) {
            printf("Error XML attribute parsing Name: %s with Value%s\n", name.c_str(), value.c_str());
            //            this->printXMLAttributeErrorMessage(element, name, value);
        }
    }
}

bool UIView::applyXMLAttribute(const std::string& name, const std::string& value) {
    REGISTER_XIB_ATTRIBUTE(contentMode, valueToContentMode, setContentMode)
    REGISTER_XIB_ATTRIBUTE(clipsToBounds, valueToBool, setClipsToBounds)
    REGISTER_XIB_ATTRIBUTE(positionType, valueToPositionType, _yoga->setPositionType)
//    REGISTER_XIB_ATTRIBUTE(isTransparentTouch, valueToBool, setTransparentTouch)
    REGISTER_XIB_ATTRIBUTE(preservesSuperviewLayoutMargins, valueToBool, setPreservesSuperviewLayoutMargins)

    REGISTER_XIB_ATTRIBUTE(cornerRadius, valueToFloat, layer()->setCornerRadius)
    REGISTER_XIB_ATTRIBUTE(backgroundColor, valueToColor, setBackgroundColor)
    REGISTER_XIB_ATTRIBUTE(tintColor, valueToColor, setTintColor)
    REGISTER_XIB_ATTRIBUTE(alpha, valueToFloat, setAlpha)

    REGISTER_XIB_ATTRIBUTE(left, valueToMetric, _yoga->setLeft)
    REGISTER_XIB_ATTRIBUTE(top, valueToMetric, _yoga->setTop)
    REGISTER_XIB_ATTRIBUTE(right, valueToMetric, _yoga->setRight)
    REGISTER_XIB_ATTRIBUTE(bottom, valueToMetric, _yoga->setBottom)
    REGISTER_XIB_ATTRIBUTE(width, valueToMetric, _yoga->setWidth)
    REGISTER_XIB_ATTRIBUTE(height, valueToMetric, _yoga->setHeight)
    REGISTER_XIB_ATTRIBUTE(direction, valueToDirection, _yoga->setDirection)
    REGISTER_XIB_ATTRIBUTE(flexDirection, valueToFlexDirection, _yoga->setFlexDirection)
    REGISTER_XIB_ATTRIBUTE(grow, valueToFloat, _yoga->setFlexGrow)
    REGISTER_XIB_ATTRIBUTE(shrink, valueToFloat, _yoga->setFlexShrink)
    REGISTER_XIB_ATTRIBUTE(wrap, valueToWrap, _yoga->setFlexWrap)
    REGISTER_XIB_ATTRIBUTE(justifyContent, valueToJustify, _yoga->setJustifyContent)
    REGISTER_XIB_ATTRIBUTE(alignItems, valueToAlign, _yoga->setAlignItems)
    REGISTER_XIB_ATTRIBUTE(alignSelf, valueToAlign, _yoga->setAlignSelf)
    REGISTER_XIB_ATTRIBUTE(alignContent, valueToAlign, _yoga->setAlignContent)
    REGISTER_XIB_ATTRIBUTE(aspectRatio, valueToFloat, _yoga->setAspectRatio)
    REGISTER_XIB_ATTRIBUTE(gap, valueToFloat, _yoga->setAllGap)

    REGISTER_XIB_EDGE_ATTRIBUTE(padding, valueToMetric, _yoga->setPadding)
    REGISTER_XIB_EDGE_ATTRIBUTE(margin, valueToMetric, _yoga->setMargin)

//    REGISTER_XIB_ATTRIBUTE(topEdgeRespects, valueToEdgeRespects, setTopEdgeRespects)
//    REGISTER_XIB_ATTRIBUTE(leftEdgeRespects, valueToEdgeRespects, setLeftEdgeRespects)
//    REGISTER_XIB_ATTRIBUTE(rightEdgeRespects, valueToEdgeRespects, setRightEdgeRespects)
//    REGISTER_XIB_ATTRIBUTE(bottomEdgeRespects, valueToEdgeRespects, setBottomEdgeRespects)

    return false;
}

UIView::UIView(NXRect frame, std::shared_ptr<CALayer> layer) {
    _yoga = new_shared<YGLayout>(shared_from_this());
//    _yoga->setEnabled(true);

    _layer = std::move(layer);
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
    setNeedsUpdateSafeAreaInsets();
}

void UIView::setBounds(NXRect bounds) {
    if (this->bounds().size != bounds.size) {
        setNeedsDisplay();
        setNeedsLayout();
        setNeedsUpdateSafeAreaInsets();
    }
    _layer->setBounds(bounds);
}

void UIView::setHidden(bool hidden) {
    _layer->setHidden(hidden);
    _yoga->setIncludedInLayout(!hidden);
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

void UIView::setInsetsLayoutMarginsFromSafeArea(bool insetsLayoutMarginsFromSafeArea) {
    if (_insetsLayoutMarginsFromSafeArea == insetsLayoutMarginsFromSafeArea) return;
    _insetsLayoutMarginsFromSafeArea = insetsLayoutMarginsFromSafeArea;
    setNeedsLayout();
}

void UIView::setPreservesSuperviewLayoutMargins(bool preservesSuperviewLayoutMargins) {
    if (_preservesSuperviewLayoutMargins == preservesSuperviewLayoutMargins) return;
    _preservesSuperviewLayoutMargins = preservesSuperviewLayoutMargins;
    setNeedsLayout();
}

UIEdgeInsets UIView::layoutMargins() {
    return _calculatedLayoutMargins;
}

void UIView::setLayoutMargins(UIEdgeInsets layoutMargins) {
    if (_layoutMargins == layoutMargins) return;
    _layoutMargins = layoutMargins;
    setNeedsUpdateLayoutMargins();
    setNeedsLayout();
}

void UIView::setSafeAreaInsets(UIEdgeInsets safeAreaInsets) {
    if (_safeAreaInsets == safeAreaInsets) return;
    _safeAreaInsets = safeAreaInsets;
    setNeedsUpdateLayoutMargins();
    updateSafeAreaInsetsInChilds();
    safeAreaInsetsDidChange();
    setNeedsLayout();
}

void UIView::updateSafeAreaInsetsInChilds() {
    for (auto& subview: _subviews) {
        subview->setNeedsUpdateSafeAreaInsets();
    }
}

void UIView::updateSafeAreaInsetsIfNeeded() {
    if (_needsUpdateSafeAreaInsets) {
        _needsUpdateSafeAreaInsets = false;
        updateSafeAreaInsets();
    }
}

void UIView::updateSafeAreaInsets() {
    if (_superview.expired()) {
        if (shared_from_base<UIWindow>() != nullptr) return;
        else return setSafeAreaInsets(UIEdgeInsets::zero);
    }

    auto parentSafeArea = _superview.lock()->_safeAreaInsets;
    auto parentSize = _superview.lock()->bounds().size;

    layoutIfNeeded();
    auto frame = this->frame();


    auto newSafeArea = UIEdgeInsets(fmaxf(0, parentSafeArea.top - fmaxf(0, frame.minY())),
                                    fmaxf(0, parentSafeArea.left - fmaxf(0, frame.minX())),
                                    fmaxf(0, parentSafeArea.bottom - fmaxf(0, (parentSize.height - frame.maxY()))),
                                    fmaxf(0, parentSafeArea.right - fmaxf(0, (parentSize.width - frame.maxX()))));

    if (!_parentController.expired()) {
        newSafeArea += _parentController.lock()->additionalSafeAreaInsets();
    }

    setSafeAreaInsets(newSafeArea);
}

void UIView::updateLayoutMarginIfNeeded() {
    if (_needsUpdateLayoutMargins) {
        _needsUpdateLayoutMargins = false;
        updateLayoutMargin();
    }
}

void UIView::updateLayoutMargin() {
    auto margins = _layoutMargins;

    bool needsSuperviewMargins = _preservesSuperviewLayoutMargins && !superview().expired();
    if (needsSuperviewMargins && _insetsLayoutMarginsFromSafeArea) {
        auto superviewMargins = superview().lock()->layoutMargins();
        auto maxCombination = UIEdgeInsets(fmaxf(_safeAreaInsets.top, superviewMargins.top),
                                           fmaxf(_safeAreaInsets.left, superviewMargins.left),
                                           fmaxf(_safeAreaInsets.bottom, superviewMargins.bottom),
                                           fmaxf(_safeAreaInsets.right, superviewMargins.right));
        margins += maxCombination;
    } else {
        if (_insetsLayoutMarginsFromSafeArea) {
            margins += _safeAreaInsets;
        }

        if (needsSuperviewMargins) {
            margins += superview().lock()->layoutMargins();
        }
    }

    if (!_parentController.expired() && _parentController.lock()->viewRespectsSystemMinimumLayoutMargins()) {
        auto minMargins = _parentController.lock()->systemMinimumLayoutMargins();
        margins = UIEdgeInsets(fmaxf(margins.top, minMargins.top),
                               fmaxf(margins.left, minMargins.left),
                               fmaxf(margins.bottom, minMargins.bottom),
                               fmaxf(margins.right, minMargins.right));
    }

    if (_calculatedLayoutMargins != margins) {
        _calculatedLayoutMargins = margins;
        layoutMarginsDidChange();
    }
}

void UIView::addGestureRecognizer(const std::shared_ptr<UIGestureRecognizer>& gestureRecognizer) {
    gestureRecognizer->_view = weak_from_this();
    _gestureRecognizers.push_back(gestureRecognizer);
}

void UIView::addSubview(const std::shared_ptr<UIView>& view) {
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

    view->setNeedsUpdateSafeAreaInsets();
}

std::shared_ptr<UIWindow> UIView::window() {
    if (!_superview.expired()) return _superview.lock()->window();
    return nullptr;
}

void UIView::setSuperview(const std::shared_ptr<UIView>& superview) {
    _superview = superview;

    if (superview)
        traitCollectionDidChange(superview->traitCollection());

    if (!_tintColor.has_value())
        tintColorDidChange();
}

void UIView::insertSubviewAt(const std::shared_ptr<UIView>& view, int index) {
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

    _layer->insertSublayerAt(view->_layer, index);
    _subviews.insert(_subviews.begin() + index, view);
    view->setSuperview(this->shared_from_this());
    view->setNeedsUpdateSafeAreaInsets();
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
    view->setNeedsUpdateSafeAreaInsets();
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

    auto tint = tintColor();

    UITraitCollection::setCurrent(traitCollection());
    auto oldTint = UIColor::_currentTint;
    UIColor::_currentTint = tint;

    if (_contentMode == UIViewContentMode::redraw) {
        if (visibleLayer->contents() && ((visibleLayer->contents()->size() / visibleLayer->contentsScale()) != visibleLayer->bounds().size || visibleLayer->scaleModifier() != layer()->scaleModifier()))
        {
            setNeedsDisplay();
        }
    }

    UITraitCollection::setCurrent(traitCollection());
    UIColor::_currentTint = tint;

    if (visibleLayer->_needsDisplay) {
        visibleLayer->display();
        visibleLayer->_needsDisplay = false;
    }

    UITraitCollection::setCurrent(traitCollection());
    UIColor::_currentTint = tint;

    if (_needsDisplay) {
        draw();
        _needsDisplay = false;
    }

    UITraitCollection::setCurrent(traitCollection());
    UIColor::_currentTint = tint;

    updateSafeAreaInsetsIfNeeded();
    updateLayoutMarginIfNeeded();
    layoutIfNeeded();

    for (auto& subview: _subviews) {
        subview->drawAndLayoutTreeIfNeeded();
    }

    UIColor::_currentTint = oldTint;
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

void UIView::setTintColor(std::optional<UIColor> tintColor) {
    if (_tintColor == tintColor) return;
    _tintColor = std::move(tintColor);
    tintColorDidChange();
}

UIColor UIView::tintColor() const {
    if (_tintColor.has_value() && _tintColor != UIColor::tint) return _tintColor.value();
    if (!superview().expired()) return superview().lock()->tintColor();
    return UIColor::systemBlue;
}

void UIView::tintColorDidChange() {
    setNeedsDisplay();
    for (const auto& child : subviews()) {
        if (!child->_tintColor.has_value())
            child->tintColorDidChange();
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

// MARK: - Focus
std::shared_ptr<UIFocusEnvironment> UIView::parentFocusEnvironment() {
    return std::dynamic_pointer_cast<UIFocusEnvironment>(next());
}

bool UIView::isFocused() {
    auto currentFocus = window()->focusSystem()->focusedItem();
    if (currentFocus.expired()) return false;

    return currentFocus.lock() == shared_from_base<UIFocusItem>();
}

std::shared_ptr<UIFocusItem> UIView::searchForFocus() {
    if (canBecomeFocused()) { return shared_from_this(); }

    if (!preferredFocusEnvironments().empty()) {
        auto res = std::dynamic_pointer_cast<UIFocusItem>(preferredFocusEnvironments().front());
        auto view = std::dynamic_pointer_cast<UIView>(res);
        if (view) return view->searchForFocus();
    }

    for (auto& child: subviews()) {
        auto res = child->searchForFocus();
        if (res) return res;
    }

    return nullptr;
}

std::shared_ptr<UIFocusItem> UIView::getNextFocusItem(std::shared_ptr<UIView> current, UIFocusHeading focusHeading) {
    auto itr = std::find(subviews().cbegin(), subviews().cend(), current);

    if (itr == subviews().cend()) {
        if (superview().expired()) return nullptr;
        return superview().lock()->getNextFocusItem(shared_from_this(), focusHeading);
    }

    auto index = itr - subviews().cbegin();

    // UIFocusHeading - previous
    if (focusHeading == UIFocusHeading::previous) {
        while (--index >= 0) {
            auto focus = subviews()[index]->searchForFocus();
            if (focus) { return focus; }
        }
    }

    // UIFocusHeading - next
    if (focusHeading == UIFocusHeading::next) {
        while (++index < subviews().size()) {
            auto focus = subviews()[index]->searchForFocus();
            if (focus) { return focus; }
        }
    }

    // UIFocusHeading - up / down
    if (_yoga->flexDirection() == YGFlexDirectionColumn) {
        if (focusHeading == UIFocusHeading::up) {
            while (--index >= 0) {
                auto focus = subviews()[index]->searchForFocus();
                if (focus) { return focus; }
            }
        }

        if (focusHeading == UIFocusHeading::down) {
            while (++index < subviews().size()) {
                auto focus = subviews()[index]->searchForFocus();
                if (focus) { return focus; }
            }
        }
    }

    // UIFocusHeading - left / right
    if (_yoga->flexDirection() == YGFlexDirectionRow) {
        if (focusHeading == UIFocusHeading::left) {
            while (--index >= 0) {
                auto focus = subviews()[index]->searchForFocus();
                if (focus) { return focus; }
            }
        }

        if (focusHeading == UIFocusHeading::right) {
            while (++index < subviews().size()) {
                auto focus = subviews()[index]->searchForFocus();
                if (focus) { return focus; }
            }
        }
    }

    return nullptr;
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

int UIView::_performWithoutAnimationTick = 0;

void UIView::performWithoutAnimation(const std::function<void()>& actionsWithoutAnimation) {
    _performWithoutAnimationTick++;
    actionsWithoutAnimation();
    _performWithoutAnimationTick--;
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
    if (!prototype || _performWithoutAnimationTick > 0) { return nullptr; }

    const auto& keyPath = event;
    auto beginFromCurrentState = (prototype->animationGroup->options & UIViewAnimationOptions::beginFromCurrentState) == UIViewAnimationOptions::beginFromCurrentState;

    auto state = beginFromCurrentState ? (_layer->presentationOrSelf()) : _layer;

    auto fromValue = state->value(keyPath);

    if (fromValue.has_value()) {
        return prototype->createAnimation(keyPath, fromValue.value());
    }

    return nullptr;
}

void UIView::updateCurrentEnvironment() {
    UITraitCollection::setCurrent(traitCollection());
    UIColor::_currentTint = tintColor();
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
bool UIView::isDescendantOf(const std::shared_ptr<UIView>& view) {
    if (view == nullptr) return false;
    auto parent = this;
    while (parent != nullptr) {
        if (parent == view.get()) return true;
        if (superview().expired()) return false;
        parent = superview().lock().get();
    }
    return false;
}

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

//        for (const auto &view : subviews()) {
//            view->setNeedsLayout();
//        }
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
    updateSafeAreaInsets();

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
