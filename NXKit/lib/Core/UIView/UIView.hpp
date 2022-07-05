//
//  UIView.hpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

#include "Geometry.hpp"
#include "UIColor.hpp"
#include <vector>

class UIView {
public:
    Rect frame;
    UIColor backgroundColor;
    UIColor borderColor = UIColor(0, 0, 0, 0);
    float cornerRadius = 0;
    float borderThickness = 0;

    UIView(Rect frame);
    UIView(float x, float y, float width, float height): UIView(Rect(x, y, width, height)) {}
    UIView(): UIView(Rect()) {}

    virtual void draw(NVGcontext* vgContext) {}
    void addSubview(UIView* subview);
    std::vector<UIView*> getSubviews();
    UIView* getSuperview();

private:
    friend class Application;
    std::vector<UIView*> subviews;
    UIView* superview = nullptr;

    void internalDraw(NVGcontext* vgContext);
};
