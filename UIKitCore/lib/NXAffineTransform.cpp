#include "NXAffineTransform.h"
#include "NXTransform3D.h"
#include <cmath>

using namespace NXKit;

#define RAD_PER_DEG 0.017453293f

const NXAffineTransform NXAffineTransform::identity = NXAffineTransform(1, 0, 0, 1, 0, 0);

NXAffineTransform::NXAffineTransform(): m11(1), m12(0), m21(0), m22(1), tX(0), tY(0) {}

NXAffineTransform::NXAffineTransform(NXFloat m11, NXFloat m12, NXFloat m21, NXFloat m22, NXFloat tX, NXFloat tY):
        m11(m11), m12(m12), m21(m21), m22(m22), tX(tX), tY(tY)
{ }

NXAffineTransform NXAffineTransform::translationBy(NXFloat x, NXFloat y) {
    return {
            1, 0,
            0, 1,
            x, y
    };
}

NXAffineTransform NXAffineTransform::scaleBy(NXFloat x, NXFloat y) {
    return {
            x, 0,
            0, y,
            0, 0
    };
}

NXAffineTransform NXAffineTransform::scale(NXFloat f) {
    return {
            f, 0,
            0, f,
            0, 0
    };
}

NXAffineTransform NXAffineTransform::rotationBy(NXFloat angle) {
    NXFloat radians = angle * RAD_PER_DEG;
    NXFloat c = cosf(radians);
    NXFloat s = sinf(radians);

    return {
            c, s,
            -s, c,
            0, 0
    };
}

std::optional<NXAffineTransform> NXAffineTransform::inverted() const {
    if (!std::isfinite(m11) || !std::isfinite(m12) || !std::isfinite(m21) ||
        !std::isfinite(m22) || !std::isfinite(tX) || !std::isfinite(tY)) return std::nullopt;
    const double d = double(m11) * m22 - double(m12) * m21;
    if (d == 0 || !std::isfinite(d)) return std::nullopt;
    NXAffineTransform transform(m22 / d, -m12 / d, -m21 / d, m11 / d,
        (double(m21) * tY - double(m22) * tX) / d,
        (double(m12) * tX - double(m11) * tY) / d);
    if (!std::isfinite(transform.m11) || !std::isfinite(transform.m12) || !std::isfinite(transform.m21) ||
        !std::isfinite(transform.m22) || !std::isfinite(transform.tX) || !std::isfinite(transform.tY)) return std::nullopt;
    return transform;
}

bool NXAffineTransform::isIdentity() const {
    return *this == identity;
}

bool NXAffineTransform::operator==(const NXAffineTransform& rhs) const {
    return m11 == rhs.m11 &&
           m12 == rhs.m12 &&
           m21 == rhs.m21 &&
           m22 == rhs.m22 &&
           tX == rhs.tX &&
           tY == rhs.tY;
}

NXAffineTransform NXAffineTransform::operator*(const NXAffineTransform& rhb) const {
    auto a = NXTransform3DMakeAffineTransform(*this);
    auto b = NXTransform3DMakeAffineTransform(rhb);
    return NXTransform3DGetAffineTransform(a * b);
}
