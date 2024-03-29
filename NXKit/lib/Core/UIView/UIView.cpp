//
//  UIView.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include <Core/UIView/UIView.hpp>
#include <Core/UIWindow/UIWindow.hpp>
#include <Core/Application/Application.hpp>
#include <Core/Utils/GroupTask/GroupTask.hpp>

namespace NXKit {

UIView::UIView(Rect frame) :
        backgroundColor(0, 0, 0, 0) {
    this->ygNode = YGNodeNew();
    YGNodeSetContext(this->ygNode, this);

    setPosition(frame.origin);
    setSize(frame.size);
}

UIView::~UIView() {
    animationContext.stop();

    if (isFocused())
        Application::shared()->setFocus(nullptr);

    for (auto recognizer: gestureRecognizers) {
//        delete recognizer;
    }

    for (auto view: subviews) {
//        if (!view->controller)
//            delete view;
    }

    YGNodeFree(ygNode);
}

void UIView::setPosition(Point position) {
    if (this->position == position) return;

    this->position = position;

    YGNodeStyleSetPosition(this->ygNode, YGEdgeLeft, position.x);
    YGNodeStyleSetPosition(this->ygNode, YGEdgeTop, position.y);
    setNeedsLayout();
}

void UIView::setSize(Size size) {
    if (bounds.size == size) return;
    bounds.size = size;

    if (size.width == UIView::AUTO) {
        YGNodeStyleSetWidthAuto(this->ygNode);
//        YGNodeStyleSetMinWidth(this->ygNode, YGUndefined);
    } else {
        YGNodeStyleSetWidth(this->ygNode, size.width);
//        YGNodeStyleSetMinWidth(this->ygNode, size.width);
    }

    if (size.height == UIView::AUTO) {
        YGNodeStyleSetHeightAuto(this->ygNode);
//        YGNodeStyleSetMinHeight(this->ygNode, YGUndefined);
    } else {
        YGNodeStyleSetHeight(this->ygNode, size.height);
//        YGNodeStyleSetMinHeight(this->ygNode, size.height);
    }
    setNeedsLayout();
}

void UIView::setWidth(float width) {
    bounds.size.width = width;
    if (width == UIView::AUTO) {
        YGNodeStyleSetWidthAuto(this->ygNode);
//        YGNodeStyleSetMinWidth(this->ygNode, YGUndefined);
    } else {
        YGNodeStyleSetWidth(this->ygNode, width);
//        YGNodeStyleSetMinWidth(this->ygNode, width);
    }
    setNeedsLayout();
}

void UIView::setHeight(float height) {
    if (bounds.size.height == height) return;

    bounds.size.height = height;
    if (height == UIView::AUTO) {
        YGNodeStyleSetHeightAuto(this->ygNode);
//        YGNodeStyleSetMinHeight(this->ygNode, YGUndefined);
    } else {
        YGNodeStyleSetHeight(this->ygNode, height);
//        YGNodeStyleSetMinHeight(this->ygNode, height);
    }
    setNeedsLayout();
}

void UIView::setMinWidth(float width) {
    if (bounds.size.width == width) return;

    bounds.size.width = width;
    if (width == UIView::AUTO) {
        YGNodeStyleSetMinWidth(this->ygNode, YGUndefined);
    } else {
        YGNodeStyleSetMinWidth(this->ygNode, width);
    }
    setNeedsLayout();
}

void UIView::setMinHeight(float height) {
    bounds.size.height = height;
    if (height == UIView::AUTO) {
        YGNodeStyleSetMinHeight(this->ygNode, YGUndefined);
    } else {
        YGNodeStyleSetMinHeight(this->ygNode, height);
    }
    setNeedsLayout();
}

void UIView::setMaxWidth(float width) {
    bounds.size.width = width;
    if (width == UIView::AUTO) {
        YGNodeStyleSetMaxWidth(this->ygNode, YGUndefined);
    } else {
        YGNodeStyleSetMaxWidth(this->ygNode, width);
    }
    setNeedsLayout();
}

void UIView::setMaxHeight(float height) {
    bounds.size.height = height;
    if (height == UIView::AUTO) {
        YGNodeStyleSetMaxHeight(this->ygNode, YGUndefined);
    } else {
        YGNodeStyleSetMaxHeight(this->ygNode, height);
    }
    setNeedsLayout();
}

void UIView::setPercentWidth(float width) {
    YGNodeStyleSetMinWidthPercent(this->ygNode, width);
    YGNodeStyleSetWidthPercent(this->ygNode, width);
    setNeedsLayout();
}

void UIView::setPercentHeight(float height) {
    YGNodeStyleSetMinHeightPercent(this->ygNode, height);
    YGNodeStyleSetHeightPercent(this->ygNode, height);
    setNeedsLayout();
}

void UIView::setMinPercentWidth(float width) {
    YGNodeStyleSetMinWidthPercent(this->ygNode, width);
    setNeedsLayout();
}

void UIView::setMinPercentHeight(float height) {
    YGNodeStyleSetMinHeightPercent(this->ygNode, height);
    setNeedsLayout();
}

void UIView::setMaxPercentWidth(float width) {
    YGNodeStyleSetMaxWidthPercent(this->ygNode, width);
    setNeedsLayout();
}

void UIView::setMaxPercentHeight(float height) {
    YGNodeStyleSetMaxHeightPercent(this->ygNode, height);
    setNeedsLayout();
}

Rect UIView::getFrame() {
    auto transformedBounds = bounds.applying(transform);

    auto anchorPointOffset = Point(
        transformedBounds.width() * anchorPoint.x,
        transformedBounds.height() * anchorPoint.y);

    return Rect(
        position.x - anchorPointOffset.x,
        position.y - anchorPointOffset.y,
        transformedBounds.width(),
        transformedBounds.height());
}

Rect UIView::getBounds() {
    return bounds;
}

void UIView::setBounds(Rect bounds) {
    this->bounds = bounds;
}

UIColor UIView::getTintColor() {
    if (tintColor.has_value())
        return tintColor.value();

    if (!superview.expired())
        return superview.lock()->getTintColor();

    return UIColor::systemTint;
}

void UIView::setTintColor(std::optional<UIColor> color) {
    tintColor = color;
    for (auto subview: subviews) {
        if (!subview->tintColor.has_value())
            subview->tintColorDidChange();
    }
}

void UIView::tintColorDidChange() {
    for (auto subview: subviews) {
        if (!subview->tintColor.has_value())
            subview->tintColorDidChange();
    }
}

void UIView::internalDraw(NVGcontext* vgContext) {
    if (isHidden() || alpha == 0) return;

    if (!drawingControlView.expired() && drawingControlView.lock()->shouldSkipSubviewDrawing(this)) {
        return;
    }

    auto trait = getTraitCollection();
    UITraitCollection::current = trait;

    nvgSave(vgContext);
    layoutIfNeeded();

    // Frame position
    nvgTranslate(vgContext, getFrame().origin.x, getFrame().origin.y);

    // Position transform
    nvgTransform(vgContext, transform.m11, transform.m12, transform.m21, transform.m22, transform.tX, transform.tY);

    // Scale transform
    // TODO: Need to be replaced with anchor point
    float scaleTransformX = (getFrame().size.width / 2.0f) - (getFrame().size.width / 2.0f) * transform.m11;
    float scaleTransformY = (getFrame().size.height / 2.0f) - (getFrame().size.height / 2.0f) * transform.m22;
    nvgTranslate(vgContext, scaleTransformX, scaleTransformY);

    nvgGlobalAlpha(vgContext, getInternalAlpha());

    // ClipToBounds
    if (clipToBounds)
        nvgIntersectScissor(vgContext, 0, 0, getFrame().size.width, getFrame().size.height);

    if (showShadow)
        drawShadow(vgContext);

    drawHighlight(vgContext, true);

    // Background color
    nvgBeginPath(vgContext);
    nvgRoundedRect(vgContext, 0, 0, getFrame().size.width, getFrame().size.height, cornerRadius);
    nvgFillColor(vgContext, backgroundColor.raw());
    nvgFill(vgContext);

    if (clickAlpha > 0) {
        nvgFillColor(vgContext, UIColor(13, 182, 213, 38).withAlphaComponent(clickAlpha).raw());
        nvgBeginPath(vgContext);
        nvgRoundedRect(vgContext, 0, 0, getBounds().width(), getBounds().height(), this->cornerRadius);
        nvgFill(vgContext);
    }

    nvgTranslate(vgContext, -bounds.minX(), -bounds.minY());
    draw(vgContext);

    // Borders
    if (!shouldDrawHighlight()) {
        if (borderThickness > 0) {
            float offset = borderThickness / 2;
            Rect borderRect = Rect(offset, offset, getFrame().size.width - offset * 2, getFrame().size.height - offset * 2);

            nvgBeginPath(vgContext);
            nvgStrokeColor(vgContext, borderColor.raw());
            nvgStrokeWidth(vgContext, this->borderThickness);
            nvgRoundedRect(vgContext, borderRect.origin.x, borderRect.origin.y, borderRect.size.width, borderRect.size.height, this->cornerRadius);
            nvgStroke(vgContext);
        }

        if (getBorderTop() > 0) {
            nvgBeginPath(vgContext);
            nvgRect(vgContext, 0, 0, getFrame().size.width, getBorderTop());
            nvgFillColor(vgContext, borderColor.raw());
            nvgFill(vgContext);
        }

        if (getBorderLeft() > 0) {
            nvgBeginPath(vgContext);
            nvgRect(vgContext, 0, 0, getBorderLeft(), getFrame().size.height);
            nvgFillColor(vgContext, borderColor.raw());
            nvgFill(vgContext);
        }

        if (getBorderRight() > 0) {
            nvgBeginPath(vgContext);
            nvgRect(vgContext, (getFrame().size.width - getBorderRight()), 0, getBorderRight(), getFrame().size.height);
            nvgFillColor(vgContext, borderColor.raw());
            nvgFill(vgContext);
        }

        if (getBorderBottom() > 0) {
            nvgBeginPath(vgContext);
            nvgRect(vgContext, 0, (getFrame().size.height - getBorderBottom()), getFrame().size.width, getBorderBottom());
            nvgFillColor(vgContext, borderColor.raw());
            nvgFill(vgContext);
        }
    }

    std::shared_ptr<UIView> focused = nullptr;
    for (auto view: subviews) {
        if (view->isFocused())
            focused = view;

        view->internalDraw(vgContext);
    }
    if (focused) {
        focused->drawHighlight(vgContext, false);
    }

    nvgRestore(vgContext);
}

static int shakeAnimation(float t, float a) // a = amplitude
{
    // Damped sine wave
    float w = 0.8f; // period
    float c = 0.35f; // damp factor
    return static_cast<int>(roundf(a * exp(-(c * t)) * sin(w * t)));
}

void UIView::shakeHighlight(NavigationDirection direction) {
    this->highlightShaking = true;
    this->highlightShakeStart = getCPUTimeUsec() / 1000;
    this->highlightShakeDirection = direction;
    this->highlightShakeAmplitude = std::rand() % 15 + 10;
}

bool UIView::shouldDrawHighlight() {
    return !(!isFocused() || !highlightOnFocus || Application::shared()->getInputType() == ApplicationInputType::TOUCH);
}

void UIView::drawShadow(NVGcontext* vg) {
    float shadowWidth = 0.0f;
    float shadowFeather = 0.0f;
    float shadowOpacity = 0.0f;
    float shadowOffset = 0.0f;

    Rect frame = getBounds();

//    switch (this->shadowType)
//    {
//        case ShadowType::GENERIC:
    shadowWidth = 2;
    shadowFeather = 10;
    shadowOpacity = 63.7f;
    shadowOffset = 10;
//            break;
//        case ShadowType::CUSTOM:
//            break;
//        case ShadowType::NONE:
//            break;
//    }

    NVGpaint shadowPaint = nvgBoxGradient(
            vg,
            frame.minX(), frame.minY() + shadowWidth,
            frame.width(), frame.height(),
            this->cornerRadius * 2, shadowFeather,
            UIColor(0, 0, 0, shadowOpacity).raw(), UIColor::clear.raw());

    nvgBeginPath(vg);
    nvgRect(
            vg,
            frame.minX() - shadowOffset,
            frame.minY() - shadowOffset,
            frame.width() + shadowOffset * 2,
            frame.height() + shadowOffset * 3);
    nvgRoundedRect(vg, frame.minX(), frame.minY(), frame.width(), frame.height(), this->cornerRadius);
    nvgPathWinding(vg, NVG_HOLE);
    nvgFillPaint(vg, shadowPaint);
    nvgFill(vg);
}

void UIView::drawHighlight(NVGcontext* vg, bool background) {
    if (!shouldDrawHighlight()) return;

    nvgSave(vg);
    nvgResetScissor(vg);

    float padding = highlightSpacing;
    float cornerRadius = highlightCornerRadius;
    float strokeWidth = 5;

    auto frame = getFrame();

    float x = -padding - strokeWidth / 2;
    float y = -padding - strokeWidth / 2;
    float width = frame.width() + padding * 2 + strokeWidth;
    float height = frame.height() + padding * 2 + strokeWidth;


    // Shake animation
    if (this->highlightShaking) {
        Time curTime = getCPUTimeUsec() / 1000;
        Time t = (curTime - highlightShakeStart) / 10;

        if (t >= 150000) {
            this->highlightShaking = false;
        } else {
            switch (this->highlightShakeDirection) {
                case NavigationDirection::RIGHT:
                    x += shakeAnimation(t, this->highlightShakeAmplitude);
                    break;
                case NavigationDirection::LEFT:
                    x -= shakeAnimation(t, this->highlightShakeAmplitude);
                    break;
                case NavigationDirection::DOWN:
                    y += shakeAnimation(t, this->highlightShakeAmplitude);
                    break;
                case NavigationDirection::UP:
                    y -= shakeAnimation(t, this->highlightShakeAmplitude);
                    break;
            }
        }
    }

    // Draw
    if (background) {
        // Background
        if (drawBackgroundOnHighlight) {
            UIColor highlightBackgroundColor = UIColor::tetriarySystemBackground;
            nvgFillColor(vg, highlightBackgroundColor.raw());
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x, y, width, height, cornerRadius);
            nvgFill(vg);
        }
    } else {
        x += frame.minX();
        y += frame.minY();

        float shadowOffset = 10;

        // Shadow
        NVGpaint shadowPaint = nvgBoxGradient(vg,
                x, y + 2,
                width, height,
                cornerRadius * 2, 10,
                UIColor(0, 0, 0, 128).raw(), UIColor::clear.raw());

        nvgBeginPath(vg);
        nvgRect(vg, x - shadowOffset, y - shadowOffset,
                width + shadowOffset * 2, height + shadowOffset * 3);
        nvgRoundedRect(vg, x, y, width, height, cornerRadius);
        nvgPathWinding(vg, NVG_HOLE);
        nvgFillPaint(vg, shadowPaint);
        nvgFill(vg);

        // Border
        float gradientX, gradientY, color;
        getHighlightAnimation(&gradientX, &gradientY, &color);

        UIColor highlightColor1 = UIColor(13, 182, 213, 25, 138, 198);

        UIColor pulsationColor = UIColor((color * highlightColor1.r()) + (1 - color) * highlightColor1.r(),
                (color * highlightColor1.g()) + (1 - color) * highlightColor1.g(),
                (color * highlightColor1.b()) + (1 - color) * highlightColor1.b());

        UIColor borderColor = UIColor(100, 255, 225, 150);

        float strokeWidth = 5;

        NVGpaint border1Paint = nvgRadialGradient(vg,
                x + gradientX * width, y + gradientY * height,
                strokeWidth * 10, strokeWidth * 40,
                borderColor.raw(), UIColor::clear.raw());

        NVGpaint border2Paint = nvgRadialGradient(vg,
                x + (1 - gradientX) * width, y + (1 - gradientY) * height,
                strokeWidth * 10, strokeWidth * 40,
                borderColor.raw(), UIColor::clear.raw());

        nvgBeginPath(vg);
        nvgStrokeColor(vg, pulsationColor.raw());
        nvgStrokeWidth(vg, strokeWidth);
        nvgRoundedRect(vg, x, y, width, height, cornerRadius);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgStrokePaint(vg, border1Paint);
        nvgStrokeWidth(vg, strokeWidth);
        nvgRoundedRect(vg, x, y, width, height, cornerRadius);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgStrokePaint(vg, border2Paint);
        nvgStrokeWidth(vg, strokeWidth);
        nvgRoundedRect(vg, x, y, width, height, cornerRadius);
        nvgStroke(vg);
    }

    nvgRestore(vg);
}

float UIView::getInternalAlpha() {
    if (!superview.expired())
        return superview.lock()->getInternalAlpha() * alpha;
    return alpha;
}

std::deque<float> UIView::createAnimationContext() {
    std::deque<float> context;
    context.push_back(bounds.origin.x);
    context.push_back(bounds.origin.y);
    context.push_back(bounds.size.width);
    context.push_back(bounds.size.height);
    context.push_back(position.x);
    context.push_back(position.y);
    context.push_back(clickAlpha);
    context.push_back(alpha);
    transform.fillAnimationContext(&context);
    backgroundColor.fillAnimationContext(&context);
    return context;
}

void UIView::applyAnimationContext(std::deque<float>* context) {
    IFNNULL(bounds.origin.x, pop(context))
    IFNNULL(bounds.origin.y, pop(context))
    IFNNULL(bounds.size.width, pop(context))
    IFNNULL(bounds.size.height, pop(context))
    IFNNULL(position.x, pop(context))
    IFNNULL(position.y, pop(context))
    IFNNULL(clickAlpha, pop(context))
    IFNNULL(alpha, pop(context))
    transform.apply(context);
    backgroundColor.apply(context);
}

void UIView::animate(float duration, std::function<void()> animations, EasingFunction easing, std::function<void(bool)> completion) {
    if (duration <= 0 || noAnimations) {
        animations();
        animationContext.reset(createAnimationContext());
        completion(true);
        return;
    }

    auto oldContext = createAnimationContext();
    animations();
    auto newContext = createAnimationContext();

    for (int i = 0; i < newContext.size(); i++) {
        if (oldContext[i] == newContext[i]) {
            oldContext[i] = std::numeric_limits<float>::quiet_NaN();
            newContext[i] = std::numeric_limits<float>::quiet_NaN();
        }
    }

    animationContext.reset(oldContext);
    animationContext.addStep(newContext, duration * 1000, easing);
    animationContext.setTickCallback([this]() {
        auto values = animationContext.getValue();
        applyAnimationContext(&values);
    });
    animationContext.setEndCallback(completion);
    applyAnimationContext(&oldContext);
    animationContext.start();
}

void UIView::setNeedsLayout() {
    YGNode* parentNode = YGNodeGetParent(this->ygNode);
    if (!parentNode) {
        needsLayout = true;
        return;
    }

    UIView* nodeHolder = (UIView*) YGNodeGetParent(this->ygNode)->getContext();
    if (!nodeHolder) {
        needsLayout = true;
        return;
    }

    nodeHolder->setNeedsLayout();
}

void UIView::layoutIfNeeded() {
    YGNode* parentNode = YGNodeGetParent(this->ygNode);
    if (!parentNode) {
        if (needsLayout)
            layoutSubviews();
        return;
    }

    UIView* nodeHolder = (UIView*) YGNodeGetParent(this->ygNode)->getContext();
    if (!nodeHolder) {
        if (needsLayout)
            layoutSubviews();
        return;
    }

    if (nodeHolder->needsLayout)
        nodeHolder->layoutSubviews();
}

void UIView::layoutSubviews() {
    if (!controller.expired()) controller.lock()->viewWillLayoutSubviews();

    needsLayout = false;

    if (YGNodeHasMeasureFunc(this->ygNode))
        YGNodeMarkDirty(this->ygNode);

    if (!YGNodeGetParent(this->ygNode)) {
        YGNodeCalculateLayout(this->ygNode, YGUndefined, YGUndefined, YGDirectionLTR);
    }

    position = Point(YGNodeLayoutGetLeft(this->ygNode), YGNodeLayoutGetTop(this->ygNode));
    bounds.size = Size(YGNodeLayoutGetWidth(this->ygNode), YGNodeLayoutGetHeight(this->ygNode));

    for (auto view: getSubviews()) {
        auto viewParentNode = YGNodeGetParent(view->ygNode);
        if (!viewParentNode || viewParentNode != this->ygNode) continue;
        view->layoutSubviews();
    }

    if (!controller.expired()) controller.lock()->viewDidLayoutSubviews();
}

void UIView::parentHierarchyViewChanged() {
    if (!superview.expired())
        drawingControlView = superview.lock()->canSkipSubviewDrawing();

    for (auto view: subviews) {
        view->parentHierarchyViewChanged();
    }
}

std::weak_ptr<UIView> UIView::canSkipSubviewDrawing() {
    if (!superview.expired())
        return superview.lock()->canSkipSubviewDrawing();
    return std::weak_ptr<UIView>();
}

bool UIView::shouldSkipSubviewDrawing(UIView* subview) {
    return false;
}

bool UIView::isFocused() {
    if (Application::shared()->getFocus().expired()) return false;
    return Application::shared()->getFocus().lock().get() == this;
}

std::shared_ptr<UIView> UIView::getDefaultFocus() {
    if (canBecomeFocused()) return shared_from_this();

    for (auto view: subviews) {
        if (view->isHidden()) continue;
        std::shared_ptr<UIView> focus = view->getDefaultFocus();
        if (focus) return focus;
    }

    return nullptr;
}

void UIView::subviewFocusDidChange(std::shared_ptr<UIView> focusedView, std::shared_ptr<UIView> notifiedView) {
    if (!superview.expired())
        superview.lock()->subviewFocusDidChange(focusedView, shared_from_this());
}

std::shared_ptr<UIView> UIView::getNextFocus(NavigationDirection direction) {
    if (superview.expired()) return nullptr;
    return superview.lock()->getNextFocus(direction);
}

void UIView::addGestureRecognizer(std::shared_ptr<UIGestureRecognizer> gestureRecognizer) {
    gestureRecognizer->view = weak_from_this();
    gestureRecognizers.push_back(gestureRecognizer);
}

std::vector<std::shared_ptr<UIGestureRecognizer>> UIView::getGestureRecognizers() {
    return gestureRecognizers;
}

void UIView::addSubview(std::shared_ptr<UIView> view) {
    view->setSuperview(weak_from_this());
    subviews.push_back(view);
    setNeedsLayout();
}

void UIView::insertSubview(std::shared_ptr<UIView> view, int position) {
    view->setSuperview(weak_from_this());
    subviews.insert(subviews.begin() + position, view);
    setNeedsLayout();
}

std::shared_ptr<UIResponder> UIView::getNext() {
    if (!controller.expired()) return controller.lock();
    if (!superview.expired()) return superview.lock();
    return nullptr;
}


void UIView::setHidden(bool hidden) {
    if (isHidden() == hidden) return;

    if (hidden)
        YGNodeStyleSetDisplay(this->ygNode, YGDisplayNone);
    else
        YGNodeStyleSetDisplay(this->ygNode, YGDisplayFlex);
    setNeedsLayout();
}

bool UIView::isHidden() {
    return YGNodeStyleGetDisplay(this->ygNode) == YGDisplayNone;
}

std::vector<std::shared_ptr<UIView>> UIView::getSubviews() {
    return subviews;
}

void UIView::removeFromSuperview() {
    if (superview.expired()) return;

    superview.lock()->subviews.erase(std::remove(superview.lock()->subviews.begin(), superview.lock()->subviews.end(), shared_from_this()));
    YGNodeRemoveChild(superview.lock()->ygNode, ygNode);
    superview.reset();
}

Point UIView::convert(Point point, std::shared_ptr<UIView> toView) {
    Point selfAbsoluteOrigin;
    Point otherAbsoluteOrigin;

    std::shared_ptr<UIView> current = shared_from_this();
    while (current) {
        if (current == toView) {
            return point + selfAbsoluteOrigin;
        }
        selfAbsoluteOrigin += current->getFrame().origin;
        selfAbsoluteOrigin -= current->bounds.origin;
        current = current->superview.lock();
    }

    current = toView;
    while (current) {
        otherAbsoluteOrigin += current->getFrame().origin;
        otherAbsoluteOrigin -= current->bounds.origin;
        current = current->superview.lock();
    }

    Point originDifference = otherAbsoluteOrigin - selfAbsoluteOrigin;
    return point - originDifference;
}

std::shared_ptr<UIView> UIView::hitTest(Point point, UIEvent* withEvent) {
    if (isHidden() || !isUserInteractionEnabled || alpha == 0)
        return nullptr;

    if (!this->point(point, withEvent))
        return nullptr;

    auto subviews = getSubviews();
    for (int i = (int) subviews.size() - 1; i >= 0; i--) {
        Point convertedPoint = shared_from_this()->convert(point, subviews[i]);
        std::shared_ptr<UIView> test = subviews[i]->hitTest(convertedPoint, withEvent);
        if (test) return test;
    }

    return shared_from_this();
}

bool UIView::point(Point insidePoint, UIEvent* withEvent) {
    return bounds.contains(insidePoint);
}

UIEdgeInsets UIView::safeAreaInsets() {
    auto insets = UIEdgeInsets::zero;
    if (!controller.expired() && controller.lock()->getParent()) {
        insets += controller.lock()->getParent()->getAdditionalSafeAreaInsets();
    }
    if (!superview.expired()) {
        insets += superview.lock()->safeAreaInsets();

        if (getFrame().origin.y > 0 && insets.top > 0) {
            insets.top -= getFrame().origin.y;
            if (insets.top < 0) insets.top = 0;
        }

        if (getFrame().origin.x > 0 || insets.left > 0) {
            insets.left -= getFrame().origin.x;
            if (insets.left < 0) insets.left = 0;
        }

        auto right = superview.lock()->getBounds().size.width - getFrame().origin.x - getBounds().size.width;
        if (right > 0 || insets.right > 0) {
            insets.right -= right;
            if (insets.right < 0) insets.right = 0;
        }

        auto bottom = superview.lock()->getBounds().size.height - getFrame().origin.y - getBounds().size.height;
        if (bottom > 0 || insets.bottom > 0) {
            insets.bottom -= bottom;
            if (insets.bottom < 0) insets.bottom = 0;
        }
    }

    return insets;
}

std::weak_ptr<UIWindow> UIView::getWindow() {
    auto superview = this;
    while (superview) {
        auto cast = superview->weak_from_base<UIWindow>();
        if (!cast.expired()) return cast;
        superview = superview->getSuperview().lock().get();
    }
    return std::weak_ptr<UIWindow>();
}

std::weak_ptr<UIView> UIView::getSuperview() {
    return superview;
}

void UIView::setSuperview(std::weak_ptr<UIView> view) {
    superview = view;
    tintColorDidChange();
    for (auto view: subviews) {
        view->parentHierarchyViewChanged();
    }
}

void UIView::setGrow(float grow) {
    YGNodeStyleSetFlexGrow(this->ygNode, grow);
}

float UIView::getGrow() {
    return YGNodeStyleGetFlexGrow(this->ygNode);
}

void UIView::setShrink(float grow) {
    YGNodeStyleSetFlexShrink(this->ygNode, grow);
}

float UIView::getShrink() {
    return YGNodeStyleGetFlexShrink(this->ygNode);
}

void UIView::setMargins(float top, float left, float bottom, float right) {
    setMarginTop(top);
    setMarginRight(right);
    setMarginBottom(bottom);
    setMarginLeft(left);
}

void UIView::setMargins(float margins) {
    setMargins(margins, margins, margins, margins);
}

void UIView::setMargins(UIEdgeInsets margins) {
    if (getMargins() == margins) return;
    setMargins(margins.top, margins.right, margins.bottom, margins.left);
}

void UIView::setMarginTop(float top) {
    if (getMargins().top == top) return;

    if (top == UIView::AUTO)
        YGNodeStyleSetMarginAuto(this->ygNode, YGEdgeTop);
    else
        YGNodeStyleSetMargin(this->ygNode, YGEdgeTop, top);

    this->setNeedsLayout();
}

void UIView::setMarginRight(float right) {
    if (getMargins().right == right) return;

    if (right == UIView::AUTO)
        YGNodeStyleSetMarginAuto(this->ygNode, YGEdgeRight);
    else
        YGNodeStyleSetMargin(this->ygNode, YGEdgeRight, right);

    this->setNeedsLayout();
}

void UIView::setMarginBottom(float bottom) {
    if (getMargins().bottom == bottom) return;

    if (bottom == UIView::AUTO)
        YGNodeStyleSetMarginAuto(this->ygNode, YGEdgeBottom);
    else
        YGNodeStyleSetMargin(this->ygNode, YGEdgeBottom, bottom);

    this->setNeedsLayout();
}

void UIView::setMarginLeft(float left) {
    if (getMargins().left == left) return;

    if (left == UIView::AUTO)
        YGNodeStyleSetMarginAuto(this->ygNode, YGEdgeLeft);
    else
        YGNodeStyleSetMargin(this->ygNode, YGEdgeLeft, left);

    this->setNeedsLayout();
}

UIEdgeInsets UIView::getMargins() {
    return UIEdgeInsets(YGNodeLayoutGetMargin(ygNode, YGEdgeTop),
            YGNodeLayoutGetMargin(ygNode, YGEdgeLeft),
            YGNodeLayoutGetMargin(ygNode, YGEdgeBottom),
            YGNodeLayoutGetMargin(ygNode, YGEdgeRight));
}

void UIView::setBorderTop(float top) {
    YGNodeStyleSetBorder(this->ygNode, YGEdgeTop, top);
}

void UIView::setBorderLeft(float left) {
    YGNodeStyleSetBorder(this->ygNode, YGEdgeLeft, left);
}

void UIView::setBorderRight(float right) {
    YGNodeStyleSetBorder(this->ygNode, YGEdgeRight, right);
}

void UIView::setBorderBottom(float bottom) {
    YGNodeStyleSetBorder(this->ygNode, YGEdgeBottom, bottom);
}

float UIView::getBorderTop() {
    return YGNodeLayoutGetBorder(this->ygNode, YGEdgeTop);
}

float UIView::getBorderLeft() {
    return YGNodeLayoutGetBorder(this->ygNode, YGEdgeLeft);
}

float UIView::getBorderRight() {
    return YGNodeLayoutGetBorder(this->ygNode, YGEdgeRight);
}

float UIView::getBorderBottom() {
    return YGNodeLayoutGetBorder(this->ygNode, YGEdgeBottom);
}

void UIView::animate(std::initializer_list<std::shared_ptr<UIView>> views, float duration, std::function<void()> animations, EasingFunction easing, std::function<void(bool)> completion) {
    // Maybe needs improvements
    if (noAnimations) {
        animations();
        completion(true);
        return;
    }

    if (duration <= 0) {
        animations();
        for (auto view: views) {
            view->animationContext.reset(view->createAnimationContext());
        }
        completion(true);
        return;
    }

    std::vector<UIView*> _views;
    std::vector<std::deque<float>> contexts;
    for (auto view: views) {
        _views.push_back(view.get());
        auto oldContext = view->createAnimationContext();
        view->animationContext.reset(oldContext);
        contexts.push_back(oldContext);
    }

    animations();

    bool* _res = new bool(true);
    GroupTask* group = new GroupTask([completion, _res]() {
        completion(*_res);
        delete _res;
    });

    for (int i = 0; i < views.size(); i++) {
        UIView* view = _views[i];

        auto newContext = view->createAnimationContext();

        for (int j = 0; j < newContext.size(); j++) {
            if (contexts[i][j] == newContext[j]) {
                contexts[i][j] = std::numeric_limits<float>::quiet_NaN();
                newContext[j] = std::numeric_limits<float>::quiet_NaN();
            }
        }

        view->animationContext.addStep(newContext, duration * 1000, easing);

        view->animationContext.setTickCallback([view]() {
            auto values = view->animationContext.getValue();
            view->applyAnimationContext(&values);
        });

        group->enter();
        view->animationContext.setEndCallback([_res, group](bool res) {
            *_res &= res;
            group->leave();
        });

        view->applyAnimationContext(&contexts[i]);
        view->animationContext.start();
    }
}

bool UIView::noAnimations = false;

void UIView::performWithoutAnimation(std::function<void()> function) {
    bool old = noAnimations;
    noAnimations = true;
    function();
    noAnimations = old;
}

// Trait collection
UITraitCollection UIView::getTraitCollection() {
    if (!controller.expired())
        return controller.lock()->getTraitCollection();

    if (!superview.expired())
        return superview.lock()->getTraitCollection();

    return UITraitCollection();
}

}
