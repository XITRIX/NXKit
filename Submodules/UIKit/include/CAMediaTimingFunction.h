#pragma once

#include <string>
#include <memory>
#include <functional>
#include <UIViewAnimationOptions.h>

namespace NXKit {

// Note these are actually acceleration rates (explains why Fast is a smaller value than Normal)
const double UIScrollViewDecelerationRateNormal = 0.998;
const double UIScrollViewDecelerationRateFast = 0.99;

const std::string kCAMediaTimingFunctionLinear = "linear";
const std::string kCAMediaTimingFunctionEaseIn = "easeIn";
const std::string kCAMediaTimingFunctionEaseOut = "easeOut";
const std::string kCAMediaTimingFunctionEaseInEaseOut = "easeInEaseOut";
const std::string kCAMediaTimingFunctionDefault = "default";
const std::string kCAMediaTimingFunctionCustomEaseOut = "customEaseOut";
const std::string kCAMediaTimingFunctionEaseOutElastic = "easeOutElastic";

class CAMediaTimingFunction {
public:
    CAMediaTimingFunction(std::string name);
    CAMediaTimingFunction(std::function<double(double)> timing);

    static double linear(double x);
    static double easeInCubic(double x);
    static double easeOutCubic(double x);
    static double easeInQuad(double x);
    static double easeOutQuad(double x);
    static double easeInOutCubic(double x);
    static double easeOutElastic(double x);

    // from CubicBezier1D optimising away constant terms
    static double customEaseOut(double x);
    static std::shared_ptr<CAMediaTimingFunction> timingFunctionFrom(UIViewAnimationOptions options);

    float at(float x);
private:
    std::function<double(double)> timing;
};

}

