#include <Timer.h>
#include <cmath>

namespace NXKit {

double truncatingRemainderFor(double value, double dividingBy) {
    return value - dividingBy * floor(value / dividingBy);
}

double timevalInMilliseconds(timeval time) {
    return (double(time.tv_sec) * 1000.0) + (double(time.tv_usec) / 1000.0);
}

double timevalInSeconds(timeval time) {
    return (double(time.tv_sec) + double(time.tv_usec) / 1000000.0);
}

Timer::Timer(double startingAtInMilliseconds) {
    auto startTime = timeval();
    gettimeofday(&startTime, nullptr);

    auto seconds = floor(startingAtInMilliseconds / 1000.0);
    auto milliseconds = truncatingRemainderFor(startingAtInMilliseconds, 1000.0);
    startTime.tv_sec += seconds;
    startTime.tv_usec += milliseconds * 1000;

    this->startTime = startTime;
}

double Timer::getElapsedTimeInMilliseconds() {
    timeval currentTime = {0, 0};
    gettimeofday(&currentTime, nullptr);
    return std::fmax(0.001, timevalInMilliseconds(currentTime) - timevalInMilliseconds(startTime));
}

double Timer::getElapsedTimeInSeconds() {
    timeval currentTime = {0, 0};
    gettimeofday(&currentTime, nullptr);
    return std::fmax(0.000001, timevalInSeconds(currentTime) - timevalInSeconds(startTime));
}

double Timer::operator-(const Timer &rhs) const {
    return timevalInMilliseconds(this->startTime) - timevalInMilliseconds(rhs.startTime);
}

bool operator==(const Timer &c1, const Timer &c2) {
    return c1.startTime.tv_sec == c2.startTime.tv_sec;
}

bool operator!=(const Timer &c1, const Timer &c2) {
    return c1.startTime.tv_sec != c2.startTime.tv_sec;
}

bool operator>(const Timer &c1, const Timer &c2) {
    return c1.startTime.tv_sec > c2.startTime.tv_sec;
}

bool operator<(const Timer &c1, const Timer &c2) {
    return c1.startTime.tv_sec < c2.startTime.tv_sec;
}

bool operator<=(const Timer &c1, const Timer &c2) {
    return c1.startTime.tv_sec <= c2.startTime.tv_sec;
}

bool operator>=(const Timer &c1, const Timer &c2) {
    return c1.startTime.tv_sec >= c2.startTime.tv_sec;
}

}
