#include <NXSize.h>

using namespace NXKit;

NXSize::NXSize(): NXSize(0, 0) {}
NXSize::NXSize(NXFloat width, NXFloat height): width(width), height(height) {}

bool NXSize::operator==(const NXSize& rhs) const {
    return this->width == rhs.width && this->height == rhs.height;
}

NXSize NXSize::operator+(const NXSize& first) const {
    return NXSize(width + first.width, height + first.height);
}

NXSize NXSize::operator-(const NXSize& first) const {
    return NXSize(width - first.width, height - first.height);
}

NXSize& NXSize::operator+=(const NXSize& rhs) {
    this->width += rhs.width;
    this->height += rhs.height;
    return *this;
}

NXSize& NXSize::operator-=(const NXSize& rhs) {
    this->width -= rhs.width;
    this->height -= rhs.height;
    return *this;
}

NXSize NXSize::operator*(const NXFloat& first) const {
    return NXSize(width * first, height * first);
}

NXSize NXSize::operator/(const NXFloat& first) const {
    return NXSize(width / first, height / first);
}

NXSize& NXSize::operator*=(const NXFloat& rhs) {
    this->width *= rhs;
    this->height *= rhs;
    return *this;
}

NXSize& NXSize::operator/=(const NXFloat& rhs) {
    this->width /= rhs;
    this->height /= rhs;
    return *this;
}