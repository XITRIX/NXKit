#include <CAMediaTimingFunction.h>
#include <tools/SharedBase.hpp>
#include <cmath>
#include <utility>

namespace NXKit {

double CAMediaTimingFunction::linear(double x) { return x; }
double CAMediaTimingFunction::easeInCubic(double x) { return std::pow(x, 3); }
double CAMediaTimingFunction::easeOutCubic(double x) {
    auto t = x - 1;
    return ((t * t * t) + 1);
}
double CAMediaTimingFunction::easeInQuad(double x) { return pow(x, 2); }
double CAMediaTimingFunction::easeOutQuad(double x) { return x * (2 - x); }
double CAMediaTimingFunction::easeInOutCubic(double x) {
    return x < 0.5 ? 2 * (x * x) : -1 + (4 - 2 * x) * x;
}
double CAMediaTimingFunction::customEaseOut(double x) {
    auto term1 = UIScrollViewDecelerationRateNormal * 3 * x * pow(1 - x, 2);
    auto term2 = 3 * (x * x) * (1 - x);
    auto term3 = x * x * x;

    return term1 + term2 + term3;
}
double CAMediaTimingFunction::easeOutElastic(double x) {
    auto c4 = (2 * M_PI) / 3;

    return x == 0
      ? 0
      : x == 1
      ? 1
      : pow(2, -10 * x) * sin((x * 10 - 0.75) * c4) + 1;
}

CAMediaTimingFunction::CAMediaTimingFunction(const std::string& name) {
    if (name == kCAMediaTimingFunctionDefault)
        timing = CAMediaTimingFunction::easeOutCubic;
    else if (name == kCAMediaTimingFunctionLinear)
        timing = CAMediaTimingFunction::linear;
    else if (name == kCAMediaTimingFunctionEaseIn)
        timing = CAMediaTimingFunction::easeInCubic;
    else if (name == kCAMediaTimingFunctionEaseOut)
        timing = CAMediaTimingFunction::easeOutQuad;
    else if (name == kCAMediaTimingFunctionCustomEaseOut)
        timing = CAMediaTimingFunction::customEaseOut;
    else if (name == kCAMediaTimingFunctionEaseInEaseOut)
        timing = CAMediaTimingFunction::easeInOutCubic;
    else if (name == kCAMediaTimingFunctionEaseOutElastic)
        timing = CAMediaTimingFunction::easeOutElastic;
    else
        throw -1;
//        fatalError("invalid name in CAMediaTimingFunction init");
}

CAMediaTimingFunction::CAMediaTimingFunction(std::function<double(double)> timing) {
    this->timing = std::move(timing);
}

std::shared_ptr<CAMediaTimingFunction> CAMediaTimingFunction::timingFunctionFrom(UIViewAnimationOptions options) {
    if ((options & UIViewAnimationOptions::curveEaseIn) == UIViewAnimationOptions::curveEaseIn) {
        return new_shared<CAMediaTimingFunction>(kCAMediaTimingFunctionEaseIn);
    } else if ((options & UIViewAnimationOptions::curveEaseOut) == UIViewAnimationOptions::curveEaseOut) {
        return new_shared<CAMediaTimingFunction>(kCAMediaTimingFunctionEaseOut);
    } else if ((options & UIViewAnimationOptions::curveEaseInOut) == UIViewAnimationOptions::curveEaseInOut) {
        return new_shared<CAMediaTimingFunction>(kCAMediaTimingFunctionEaseInEaseOut);
    } else if ((options & UIViewAnimationOptions::customEaseOut) == UIViewAnimationOptions::customEaseOut) {
        return new_shared<CAMediaTimingFunction>(kCAMediaTimingFunctionCustomEaseOut);
    } else if ((options & UIViewAnimationOptions::curveLinear) == UIViewAnimationOptions::curveLinear) {
        return new_shared<CAMediaTimingFunction>(kCAMediaTimingFunctionLinear);
    } else if ((options & UIViewAnimationOptions::curveEaseOutElastic) == UIViewAnimationOptions::curveEaseOutElastic) {
        return new_shared<CAMediaTimingFunction>(kCAMediaTimingFunctionEaseOutElastic);
    }

    return new_shared<CAMediaTimingFunction>(kCAMediaTimingFunctionDefault);
}
float CAMediaTimingFunction::at(float x) {
    return (float) timing(x);
}

}

