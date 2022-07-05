//
//  Geometry.h
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

struct Point {
    double x, y;

    Point(): x(0), y(0) { }
    Point(double x, double y): x(x), y(y) { }
};

struct Size {
    double width, height;

    Size(): width(0), height(0) { }
    Size(double width, double height): width(width), height(height) { }
};

struct Rect {
    Point origin;
    Size size;

    Rect(): origin(), size() { }
    Rect(Point origin, Size size): origin(origin), size(size) { }
    Rect(double x, double y, double width, double height): origin(x, y), size(width, height) { }
};
