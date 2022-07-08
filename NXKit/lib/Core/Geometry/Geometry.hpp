//
//  Geometry.h
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

struct Point {
    float x, y;

    Point(): x(0), y(0) { }
    Point(float x, float y): x(x), y(y) { }
};

struct Size {
    float width, height;

    Size(): width(0), height(0) { }
    Size(float width, float height): width(width), height(height) { }
};

struct Rect {
    Point origin;
    Size size;

    Rect(): origin(), size() { }
    Rect(Point origin, Size size): origin(origin), size(size) { }
    Rect(float x, float y, float width, float height): origin(x, y), size(width, height) { }
};
