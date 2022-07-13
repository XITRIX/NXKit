//
//  UIView.hpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

#include "Geometry.hpp"
#include "CGPoint.hpp"
#include "CGSize.hpp"
#include "CGRect.hpp"
#include "UIColor.hpp"
#include "UIResponder.hpp"
#include "tweeny/tweeny.h"

#include <yoga/YGNode.h>

#include <vector>

class UIViewController;
class UIWindow;

class UIView: public UIResponder {
public:
    static constexpr float AUTO = NAN;

    CGRect frame;
    UIColor backgroundColor;
    UIColor borderColor = UIColor(0, 0, 0);
    float cornerRadius = 0;
    float borderThickness = 0;
    CGPoint transformOrigin;
    CGSize transformSize = Size(1, 1);

    UIView(Rect frame);
    UIView(float x, float y, float width, float height): UIView(Rect(x, y, width, height)) {}
    UIView(): UIView(Rect(0, 0, UIView::AUTO, UIView::AUTO)) {}

    virtual void draw(NVGcontext* vgContext) {}
    virtual void addSubview(UIView* view);
    std::vector<UIView*> getSubviews();

    UIResponder* getNext() override;

    Point convert(Point point, UIView* toView);

    UIView* hitTest(Point point, UIEvent* withEvent);
    bool point(Point insidePoint, UIEvent* withEvent);

    UIView* getSuperview();

    void animate(float duration, std::function<void()> animations, std::function<void()> completion = [](){});

    // Yoga node
    YGNode* getYGNode() { return this->ygNode; }

    // Sizes
    Rect getBounds();
    void setPosition(Point position);
    void setSize(Size size);
    void setGrow(float grow);
    float getGrow();

    // Margins
    void setMargins(float top, float right, float bottom, float left);
    void setMargins(float margins);

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
protected:
    YGNode* ygNode;

private:
    friend class Application;
    friend class UIViewController;

    UIViewController* controller;
    std::vector<UIView*> subviews;
    UIView* superview = nullptr;
    bool needsLayout = true;
    Rect bounds;

    Rect getFrame();

    void internalDraw(NVGcontext* vgContext);
    void setSuperview(UIView* view);
};
