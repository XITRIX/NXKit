#pragma once

namespace NXKit {

typedef double NXFloat;

struct NXSize {
    NXFloat width;
    NXFloat height;

    NXSize();

    NXSize(NXFloat width, NXFloat height);

    bool operator==(const NXSize &rhs) const;

    NXSize operator+(const NXSize &first) const;

    NXSize operator-(const NXSize &first) const;

    NXSize &operator+=(const NXSize &rhs);

    NXSize &operator-=(const NXSize &rhs);

    NXSize operator*(const NXFloat &first) const;

    NXSize operator/(const NXFloat &first) const;

    NXSize &operator*=(const NXFloat &rhs);

    NXSize &operator/=(const NXFloat &rhs);
};

}