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
#include <tweeny/tweeny.h>

#include <yoga/YGNode.h>

#include <vector>
#include <optional>

namespace NXKit {

class UIViewController;
class UIWindow;

enum class NavigationDirection {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

class UIView: public UIResponder, public UITraitEnvironment {
public:
    static constexpr float AUTO = NAN;
    
    static void animate(std::initializer_list<UIView*> views, float duration, std::function<void()> animations, EasingFunction easing = EasingFunction::linear, std::function<void(bool)> completion = [](bool res){});

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
    virtual void addSubview(UIView* view);
    virtual void insertSubview(UIView* view, int position);
    std::vector<UIView*> getSubviews();
    void removeFromSuperview();

    UIResponder* getNext() override;

    UIColor getTintColor();
    void setTintColor(std::optional<UIColor> color);

    Point convert(Point point, UIView* toView);

    virtual UIView* hitTest(Point point, UIEvent* withEvent);
    virtual bool point(Point insidePoint, UIEvent* withEvent);

    UIWindow* getWindow();
    UIView* getSuperview();

    virtual UIView* getDefaultFocus();
    virtual UIView* getNextFocus(NavigationDirection direction);

    bool isFocused();
    virtual void becomeFocused() {}
    virtual void resignFocused() {}
    virtual void subviewFocusDidChange(UIView* focusedView, UIView* notifiedView);
    virtual bool canBecomeFocused() { return false; }

    virtual std::deque<float> createAnimationContext();
    virtual void applyAnimationContext(std::deque<float>* context);
    void animate(float duration, std::function<void()> animations, EasingFunction easing = EasingFunction::linear, std::function<void(bool)> completion = [](bool res){});

    void addGestureRecognizer(UIGestureRecognizer* gestureRecognizer);
    std::vector<UIGestureRecognizer*> getGestureRecognizers();

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
    void setPercentWidth(float width);
    void setPercentHeight(float height);

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

    std::optional<UIColor> tintColor;

    AnimationContext animationContext;
    std::vector<UIGestureRecognizer*> gestureRecognizers;
    UIViewController* controller = nullptr;
    std::vector<UIView*> subviews;
    UIView* superview = nullptr;
    bool needsLayout = true;
    Rect bounds;

    // highlight shaking
    bool highlightShaking = false;
    Time highlightShakeStart;
    NavigationDirection highlightShakeDirection;
    float highlightShakeAmplitude;

    void internalDraw(NVGcontext* vgContext);
    void setSuperview(UIView* view);
    void drawShadow(NVGcontext* vg);
    void drawHighlight(NVGcontext* vg, bool background);

    void shakeHighlight(NavigationDirection direction);
    bool shouldDrawHighlight();

    float getInternalAlpha();
};

}
