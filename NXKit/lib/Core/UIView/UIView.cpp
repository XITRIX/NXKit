//
//  UIView.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include <Core/UIView/UIView.hpp>
#include <Core/UIWindow/UIWindow.hpp>
#include <Core/UIViewController/UIViewController.hpp>
#include <Core/Application/Application.hpp>
#include <Core/UIStackView/UIStackView.hpp>
#include <Platforms/InputManager.hpp>

namespace NXKit {

UIView::UIView(Rect frame):
backgroundColor(0, 0, 0, 0)
{
    this->ygNode = YGNodeNew();
    YGNodeSetContext(this->ygNode, this);

    setPosition(frame.origin);
    setSize(frame.size);

//    auto inputManager = InputManager::shared();
//    inputManager->getInputUpdated()->subscribe([inputManager](){
        //        inputManager->
//    });
}

UIView::~UIView() {
    for (auto recognizer: gestureRecognizers) {
        delete recognizer;
    }

    for (auto view: subviews) {
        if (!view->controller)
            delete view;
    }

    YGNodeFree(ygNode);
}

void UIView::setPosition(Point position) {
    YGNodeStyleSetPosition(this->ygNode, YGEdgeLeft, position.x);
    YGNodeStyleSetPosition(this->ygNode, YGEdgeTop, position.y);
    setNeedsLayout();
}

void UIView::setSize(Size size) {
    bounds.size = size;
    if (size.width == UIView::AUTO)
    {
        YGNodeStyleSetWidthAuto(this->ygNode);
        YGNodeStyleSetMinWidth(this->ygNode, YGUndefined);
    }
    else
    {
        YGNodeStyleSetWidth(this->ygNode, size.width);
        YGNodeStyleSetMinWidth(this->ygNode, size.width);
    }

    if (size.height == UIView::AUTO)
    {
        YGNodeStyleSetHeightAuto(this->ygNode);
        YGNodeStyleSetMinHeight(this->ygNode, YGUndefined);
    }
    else
    {
        YGNodeStyleSetHeight(this->ygNode, size.height);
        YGNodeStyleSetMinHeight(this->ygNode, size.height);
    }
    setNeedsLayout();
}

void UIView::setWidth(float width) {
    bounds.size.width = width;
    if (width == UIView::AUTO)
    {
        YGNodeStyleSetWidthAuto(this->ygNode);
        YGNodeStyleSetMinWidth(this->ygNode, YGUndefined);
    }
    else
    {
        YGNodeStyleSetWidth(this->ygNode, width);
        YGNodeStyleSetMinWidth(this->ygNode, width);
    }
    setNeedsLayout();
}

void UIView::setHeight(float height) {
    bounds.size.height = height;
    if (height == UIView::AUTO)
    {
        YGNodeStyleSetHeightAuto(this->ygNode);
        YGNodeStyleSetMinHeight(this->ygNode, YGUndefined);
    }
    else
    {
        YGNodeStyleSetHeight(this->ygNode, height);
        YGNodeStyleSetMinHeight(this->ygNode, height);
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

Rect UIView::getFrame() {
    return Rect( YGNodeLayoutGetLeft(this->ygNode), YGNodeLayoutGetTop(this->ygNode), YGNodeLayoutGetWidth(this->ygNode), YGNodeLayoutGetHeight(this->ygNode) );
}

Rect UIView::getBounds() {
    Rect bounds;
    bounds.origin = this->bounds.origin;
    bounds.size = Size(YGNodeLayoutGetWidth(this->ygNode), YGNodeLayoutGetHeight(this->ygNode));
    return bounds;
}

void UIView::setBounds(Rect bounds) {
    this->bounds = bounds;
}

void UIView::internalDraw(NVGcontext* vgContext) {
    if (isHidden()) return;

    nvgSave(vgContext);
    layoutIfNeeded();

    // Frame position
    nvgTranslate(vgContext, getFrame().origin.x , getFrame().origin.y);

    // Position transform
    nvgTranslate(vgContext, transformOrigin.x , transformOrigin.y);

    // Scale transform
    float scaleTransformX = (getFrame().size.width / 2.0f) - (getFrame().size.width / 2.0f) * transformSize.width;
    float scaleTransformY = (getFrame().size.height / 2.0f) - (getFrame().size.height / 2.0f) * transformSize.height;
    nvgTranslate(vgContext, scaleTransformX, scaleTransformY);
    nvgScale(vgContext, transformSize.width, transformSize.height);

    // ClipToBounds
    if (clipToBounds)
        nvgIntersectScissor(vgContext, 0, 0, getFrame().size.width, getFrame().size.height);

    // Background color
    nvgBeginPath(vgContext);
    nvgRoundedRect(vgContext, 0, 0, getFrame().size.width, getFrame().size.height, cornerRadius);
    nvgFillColor(vgContext, backgroundColor.raw());
    nvgFill(vgContext);
    
    drawHighlight(vgContext, true);

    nvgSave(vgContext);
    nvgTranslate(vgContext, -bounds.minX(), -bounds.minY());
    draw(vgContext);

    nvgSave(vgContext);

    UIView* focused = nullptr;
    for (auto view: subviews) {
        if (view->isFocused())
            focused = view;

        view->internalDraw(vgContext);
    }
    if (focused) {
        focused->drawHighlight(vgContext, false);
    }
    nvgRestore(vgContext);
    nvgRestore(vgContext);

    // Borders
    if (!isFocused() || !highlightOnFocus) {
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

//    drawHighlight(vgContext, false);

    nvgRestore(vgContext);
}

void UIView::drawHighlight(NVGcontext* vg, bool background) {
    if (!isFocused() || !highlightOnFocus) return;

    nvgSave(vg);
    nvgResetScissor(vg);

    float padding = 0;
    float cornerRadius = 0.5f;
    float strokeWidth  = 5;

    auto frame = getFrame();

    float width  = frame.width() + padding * 2 + strokeWidth;
    float height = frame.height() + padding * 2 + strokeWidth;

    // Draw
    if (background)
    {
        float x = padding - strokeWidth / 2;
        float y = padding - strokeWidth / 2;

        // Background
        UIColor highlightBackgroundColor = UIColor(252, 255, 248);
        nvgFillColor(vg, highlightBackgroundColor.raw());
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, width, height, cornerRadius);
        nvgFill(vg);
    }
    else
    {
        float x = frame.minX() - padding - strokeWidth / 2;
        float y = frame.minY() - padding - strokeWidth / 2;

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

        UIColor highlightColor1 = UIColor(13, 182, 213);

        UIColor pulsationColor = UIColor((color * highlightColor1.r()) + (1 - color) * highlightColor1.r(),
            (color * highlightColor1.g()) + (1 - color) * highlightColor1.g(),
            (color * highlightColor1.b()) + (1 - color) * highlightColor1.b());

        UIColor borderColor = UIColor(80, 239, 217, 150);

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

std::deque<float> UIView::createAnimationContext() {
    std::deque<float> context;
    context.push_back(bounds.origin.x);
    context.push_back(bounds.origin.y);
    context.push_back(transformOrigin.x);
    context.push_back(transformOrigin.y);
    return context;
}

void UIView::applyAnimationContext(std::deque<float>* context) {
    bounds.origin.x = pop(context);
    bounds.origin.y = pop(context);
    transformOrigin.x = pop(context);
    transformOrigin.y = pop(context);
}

void UIView::animate(float duration, std::function<void()> animations, EasingFunction easing, std::function<void(bool)> completion) {
    if (duration <= 0) {
        animations();
        animationContext.reset(createAnimationContext());
        completion(true);
        return;
    }
    
    auto oldContext = createAnimationContext();
    animationContext.reset(oldContext);
    animations();
    animationContext.addStep(createAnimationContext(), duration, easing);
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
    if (controller) controller->viewWillLayoutSubviews();

    needsLayout = false;

    if (YGNodeHasMeasureFunc(this->ygNode))
        YGNodeMarkDirty(this->ygNode);

    if (!YGNodeGetParent(this->ygNode)) {
        YGNodeCalculateLayout(this->ygNode, YGUndefined, YGUndefined, YGDirectionLTR);
    }

    for (auto view: getSubviews()) {
        auto viewParentNode = YGNodeGetParent(view->ygNode);
        if (!viewParentNode || viewParentNode != this->ygNode) continue;
        view->layoutSubviews();
    }

    bounds = getBounds();

    if (controller) controller->viewDidLayoutSubviews();
}

bool UIView::isFocused() {
    return Application::shared()->getFocus() == this;
}

UIView* UIView::getDefaultFocus() {
    if (canBecomeFocused()) return this;

    for (auto view: subviews) {
        UIView* focus = view->getDefaultFocus();
        if (focus) return focus;
    }

    return nullptr;
}

void UIView::subviewFocusDidChange(UIView* focusedView, UIView* notifiedView) {
    if (superview)
        superview->subviewFocusDidChange(focusedView, this);
}

UIView* UIView::getNextFocus(NavigationDirection direction) {
    if (!superview) return nullptr;
    return superview->getNextFocus(direction);
}

void UIView::addGestureRecognizer(UIGestureRecognizer* gestureRecognizer) {
    gestureRecognizer->view = this;
    gestureRecognizers.push_back(gestureRecognizer);
}

std::vector<UIGestureRecognizer*> UIView::getGestureRecognizers() {
    return gestureRecognizers;
}

void UIView::addSubview(UIView *view) {
    view->setSuperview(this);
    subviews.push_back(view);
    setNeedsLayout();
}

void UIView::insertSubview(UIView* view, int position) {
    view->setSuperview(this);
    subviews.insert(subviews.begin() + position, view);
    setNeedsLayout();
}

UIResponder* UIView::getNext() {
    if (controller) return controller;
    if (superview) return superview;
    return nullptr;
}


void UIView::setHidden(bool hidden) {
    if (hidden)
        YGNodeStyleSetDisplay(this->ygNode, YGDisplayNone);
    else
        YGNodeStyleSetDisplay(this->ygNode, YGDisplayFlex);
    setNeedsLayout();
}

bool UIView::isHidden() {
    return YGNodeStyleGetDisplay(this->ygNode) == YGDisplayNone;
}

std::vector<UIView*> UIView::getSubviews() {
    return subviews;
}

void UIView::removeFromSuperview() {
    if (!superview) return;
    
    superview->subviews.erase(std::remove(superview->subviews.begin(), superview->subviews.end(), this));
    YGNodeRemoveChild(superview->ygNode, ygNode);
    superview = nullptr;
}

Point UIView::convert(Point point, UIView* toView) {
    Point selfAbsoluteOrigin;
    Point otherAbsoluteOrigin;

    UIView* current = this;
    while (current) {
        if (current == toView) {
            return point - selfAbsoluteOrigin;
        }
        selfAbsoluteOrigin += current->getFrame().origin;
        selfAbsoluteOrigin -= current->bounds.origin;
        current = current->superview;
    }

    current = toView;
    while (current) {
        otherAbsoluteOrigin += current->getFrame().origin;
        otherAbsoluteOrigin -= current->bounds.origin;
        current = current->superview;
    }

    Point originDifference = otherAbsoluteOrigin - selfAbsoluteOrigin;
    return point - originDifference;
}

UIView* UIView::hitTest(Point point, UIEvent* withEvent) {
    if (!this->point(point, withEvent)) return nullptr;

    auto subviews = getSubviews();
    for (int i = (int) subviews.size()-1; i >= 0; i--) {
        Point convertedPoint = subviews[i]->convert(point, this);
        UIView* test = subviews[i]->hitTest(convertedPoint, withEvent);
        if (test) return test;
    }

    return this;
}

bool UIView::point(Point insidePoint, UIEvent *withEvent) {
    return bounds.contains(insidePoint);
}

UIEdgeInsets UIView::safeAreaInsets() {
    auto insets = UIEdgeInsets::zero;
    if (controller && controller->getParent()) {
        insets += controller->getParent()->getAdditionalSafeAreaInsets();
    }
    if (superview) {
        insets += superview->safeAreaInsets();

        if (getFrame().origin.y > 0 && insets.top > 0) {
            insets.top -= getFrame().origin.y;
            if (insets.top < 0) insets.top = 0;
        }

        if (getFrame().origin.x > 0 || insets.left > 0) {
            insets.left -= getFrame().origin.x;
            if (insets.left < 0) insets.left = 0;
        }

        auto right = superview->getBounds().size.width - getFrame().origin.x - getBounds().size.width;
        if (right > 0 || insets.right > 0) {
            insets.right -= right;
            if (insets.right < 0) insets.right = 0;
        }

        auto bottom = superview->getBounds().size.height - getFrame().origin.y - getBounds().size.height;
        if (bottom > 0 || insets.bottom > 0) {
            insets.bottom -= bottom;
            if (insets.bottom < 0) insets.bottom = 0;
        }
    }

    return insets;
}

UIWindow* UIView::getWindow() {
    auto superview = this;
    while (superview) {
        auto cast = dynamic_cast<UIWindow*>(superview);
        if (cast) return cast;
        superview = superview->getSuperview();
    }
    return nullptr;
}

UIView* UIView::getSuperview() {
    return superview;
}

void UIView::setSuperview(UIView* view) {
    superview = view;
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

void UIView::setMargins(float top, float right, float bottom, float left) {
    setMarginTop(top);
    setMarginRight(right);
    setMarginBottom(bottom);
    setMarginLeft(left);
}

void UIView::setMargins(float margins) {
    setMargins(margins, margins, margins, margins);
}

void UIView::setMarginTop(float top) {
    if (top == UIView::AUTO)
        YGNodeStyleSetMarginAuto(this->ygNode, YGEdgeTop);
    else
        YGNodeStyleSetMargin(this->ygNode, YGEdgeTop, top);

    this->setNeedsLayout();
}

void UIView::setMarginRight(float right) {
    if (right == UIView::AUTO)
        YGNodeStyleSetMarginAuto(this->ygNode, YGEdgeRight);
    else
        YGNodeStyleSetMargin(this->ygNode, YGEdgeRight, right);

    this->setNeedsLayout();
}

void UIView::setMarginBottom(float bottom) {
    if (bottom == UIView::AUTO)
        YGNodeStyleSetMarginAuto(this->ygNode, YGEdgeBottom);
    else
        YGNodeStyleSetMargin(this->ygNode, YGEdgeBottom, bottom);

    this->setNeedsLayout();
}

void UIView::setMarginLeft(float left) {
    if (left == UIView::AUTO)
        YGNodeStyleSetMarginAuto(this->ygNode, YGEdgeLeft);
    else
        YGNodeStyleSetMargin(this->ygNode, YGEdgeLeft, left);

    this->setNeedsLayout();
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

}
