#pragma once

#include <sys/time.h>

namespace NXKit {

double timevalInMilliseconds(timeval time);
double timevalInSeconds(timeval time);

struct Timer {
    timeval startTime;

    Timer(double startingAt = 0);

    double getElapsedTimeInMilliseconds();
    double getElapsedTimeInSeconds();

    double operator-(const Timer& rhs) const;

    friend bool operator== (const Timer& c1, const Timer& c2);
    friend bool operator!= (const Timer& c1, const Timer& c2);

    friend bool operator< (const Timer& c1, const Timer& c2);
    friend bool operator> (const Timer& c1, const Timer& c2);

    friend bool operator<= (const Timer& c1, const Timer& c2);
    friend bool operator>= (const Timer& c1, const Timer& c2);
};

}
