//
//  UIView.hpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

#include <Core/Geometry/Geometry.hpp>
#include <Core/Utils/Tools/Tools.hpp>
#include <Core/Utils/Literals/Literals.hpp>
#include <Core/Utils/Animation/AnimationContext/AnimationContext.hpp>
#include <Core/UIColor/UIColor.hpp>
#include <Core/UIResponder/UIResponder.hpp>
#include <Core/UIEdgeInsets/UIEdgeInsets.hpp>
#include <Core/UIGestureRecognizer/UIGestureRecognizer.hpp>
#include <Core/UITraitCollection/UITraitCollection.hpp>
#include <Core/Utils/SharedBase/SharedBase.hpp>
#include <tweeny/tweeny.h>

#include <yoga/YGNode.h>

#include <vector>
#include <optional>
#include <memory>

namespace NXKit {

class UIViewController;
class UIWindow;

enum class NavigationDirection {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

class UIView: public UIResponder, public UITraitEnvironment, public enable_shared_from_base<UIView> {
public:
    static constexpr float AUTO = NAN;
    
    static void animate(std::initializer_list<std::shared_ptr<UIView>> views, float duration, std::function<void()> animations, EasingFunction easing = EasingFunction::linear, std::function<void(bool)> completion = [](bool res){});

    std::string tag;
    UIColor backgroundColor;
    UIColor borderColor = UIColor(0, 0, 0);
    float cornerRadius = 0;
    float highlightCornerRadius = 0.5f;
    float borderThickness = 0;
    Point transformOrigin;
    Size transformSize = Size(1, 1);
    bool clipToBounds = true;
    bool highlightOnFocus = true;
    float alpha = 1;
    float clickAlpha = 0;
    bool showShadow = false;
    bool drawBackgroundOnHighlight = true;
    float highlightSpacing = 0;
    bool isUserInteractionEnabled = true;

    UIView(Rect frame);
    UIView(float x, float y, float width, float height): UIView(Rect(x, y, width, height)) {}
    UIView(): UIView(Rect(0, 0, UIView::AUTO, UIView::AUTO)) {}
    virtual ~UIView();

    virtual void draw(NVGcontext* vgContext) {}
    virtual void addSubview(std::shared_ptr<UIView> view);
    virtual void insertSubview(std::shared_ptr<UIView> view, int position);
    std::vector<std::shared_ptr<UIView>> getSubviews();
    void removeFromSuperview();

    std::shared_ptr<UIResponder> getNext() override;

    UIColor getTintColor();
    void setTintColor(std::optional<UIColor> color);
    virtual void tintColorDidChange();

    Point convert(Point point, std::shared_ptr<UIView> toView);

    virtual std::shared_ptr<UIView> hitTest(Point point, UIEvent* withEvent);
    virtual bool point(Point insidePoint, UIEvent* withEvent);

    std::weak_ptr<UIWindow> getWindow();
    std::weak_ptr<UIView> getSuperview();

    virtual std::shared_ptr<UIView> getDefaultFocus();
    virtual std::shared_ptr<UIView> getNextFocus(NavigationDirection direction);

    bool isFocused();
    virtual void becomeFocused() {}
    virtual void resignFocused() {}
    virtual void subviewFocusDidChange(std::shared_ptr<UIView> focusedView, std::shared_ptr<UIView> notifiedView);
    virtual bool canBecomeFocused() { return false; }

    virtual std::deque<float> createAnimationContext();
    virtual void applyAnimationContext(std::deque<float>* context);
    void animate(float duration, std::function<void()> animations, EasingFunction easing = EasingFunction::linear, std::function<void(bool)> completion = [](bool res){});

    static void performWithoutAnimation(std::function<void()> function);

    void addGestureRecognizer(std::shared_ptr<UIGestureRecognizer> gestureRecognizer);
    std::vector<std::shared_ptr<UIGestureRecognizer>> getGestureRecognizers();

    // Yoga node
    YGNode* getYGNode() { return this->ygNode; }

    void setHidden(bool hidden);
    bool isHidden();

    // Sizes
    Rect getFrame();
    Rect getBounds();
    virtual void setBounds(Rect bounds);
    void setPosition(Point position);
    void setGrow(float grow);
    float getGrow();
    void setShrink(float grow);
    float getShrink();

    void setSize(Size size);

    void setWidth(float width);
    void setHeight(float height);

    void setMinWidth(float width);
    void setMinHeight(float height);

    void setMaxWidth(float width);
    void setMaxHeight(float height);

    void setPercentWidth(float width);
    void setPercentHeight(float height);

    void setMinPercentWidth(float width);
    void setMinPercentHeight(float height);

    void setMaxPercentWidth(float width);
    void setMaxPercentHeight(float height);

    // Margins
    void setMargins(UIEdgeInsets margins);
    void setMargins(float top, float left, float bottom, float right);
    void setMargins(float margins);
    UIEdgeInsets getMargins();

    void setMarginTop(float top);
    void setMarginRight(float right);
    void setMarginBottom(float right);
    void setMarginLeft(float left);

    // Borders
    void setBorderTop(float top);
    void setBorderLeft(float left);
    void setBorderRight(float right);
    void setBorderBottom(float bottom);

    float getBorderTop();
    float getBorderLeft();
    float getBorderRight();
    float getBorderBottom();

    // Layout
    void setNeedsLayout();
    void layoutIfNeeded();
    virtual void layoutSubviews();

    virtual UIEdgeInsets safeAreaInsets();

    // Trait Environment
    UITraitCollection getTraitCollection() override;
protected:
    YGNode* ygNode;

private:
    friend class Application;
    friend class UIViewController;
    friend class UITableView;

    static bool noAnimations;
    std::optional<UIColor> tintColor;

    AnimationContext animationContext;
    std::vector<std::shared_ptr<UIGestureRecognizer>> gestureRecognizers;
    std::weak_ptr<UIViewController> controller;
    std::vector<std::shared_ptr<UIView>> subviews;
    std::weak_ptr<UIView> superview;
    bool needsLayout = true;
    Rect bounds;

    // highlight shaking
    bool highlightShaking = false;
    Time highlightShakeStart;
    NavigationDirection highlightShakeDirection;
    float highlightShakeAmplitude;

    void internalDraw(NVGcontext* vgContext);
    void setSuperview(std::weak_ptr<UIView> view);
    void drawShadow(NVGcontext* vg);
    void drawHighlight(NVGcontext* vg, bool background);

    void shakeHighlight(NavigationDirection direction);
    bool shouldDrawHighlight();

    float getInternalAlpha();
};

}
