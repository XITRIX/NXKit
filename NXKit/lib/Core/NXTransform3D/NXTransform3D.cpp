//
//  NXTransform3D.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 19.08.2022.
//

#include <Core/NXTransform3D/NXTransform3D.hpp>

namespace NXKit {

NXTransform3D::NXTransform3D(
    float m11, float m12, float m13, float m14,
    float m21, float m22, float m23, float m24,
    float m31, float m32, float m33, float m34,
    float m41, float m42, float m43, float m44)
:
    m11(m11), m12(m12), m13(m13), m14(m14),
    m21(m11), m22(m12), m23(m13), m24(m14),
    m31(m11), m32(m12), m33(m13), m34(m14),
    m41(m11), m42(m12), m43(m13), m44(m14)
{ }

NXTransform3D::NXTransform3D() :
    m11(0), m12(0), m13(0), m14(0),
    m21(0), m22(0), m23(0), m24(0),
    m31(0), m32(0), m33(0), m34(0),
    m41(0), m42(0), m43(0), m44(0)
{ }

NXTransform3D NXTransform3D::concat(const NXTransform3D& other) {
    return CATransform3DConcat(*this, other);
}

bool NXTransform3D::operator==(const NXTransform3D& rhs) {
    return NXTransform3DEqualToTransform(*this, rhs);
}

bool NXTransform3DEqualToTransform(NXTransform3D a, NXTransform3D b) {
    return
        a.m11 == b.m11 && a.m12 == b.m12 && a.m13 == b.m13 && a.m14 == b.m14 &&
        a.m21 == b.m21 && a.m22 == b.m22 && a.m23 == b.m23 && a.m24 == b.m24 &&
        a.m31 == b.m31 && a.m32 == b.m32 && a.m33 == b.m33 && a.m34 == b.m34 &&
        a.m41 == b.m41 && a.m42 == b.m42 && a.m43 == b.m43 && a.m44 == b.m44;
}

NXTransform3D NXTransform3DMakeAffineTransform(NXAffineTransform m) {
    return NXTransform3D(
        m.m11, m.m12, 0,    0,
        m.m21, m.m22, 0,    0,
        0,     0,     1,    0,
        m.tX,  m.tY,  0,    1);
}

NXTransform3D CATransform3DMakeTranslation(float tx, float ty, float tz) {
    return NXTransform3D(
        1,     0,     0,    0,
        0,     1,     0,    0,
        0,     0,     1,    0,
        tx,    ty,    tz,   1);
}

NXTransform3D CATransform3DConcat(NXTransform3D a, NXTransform3D b) {
    if (a == NXTransform3DIdentity) { return b; }
    if (b == NXTransform3DIdentity) { return a; }

    auto result = NXTransform3D();

    result.m11 = a.m11 * b.m11 + a.m21 * b.m12 + a.m31 * b.m13 + a.m41 * b.m14;
    result.m12 = a.m12 * b.m11 + a.m22 * b.m12 + a.m32 * b.m13 + a.m42 * b.m14;
    result.m13 = a.m13 * b.m11 + a.m23 * b.m12 + a.m33 * b.m13 + a.m43 * b.m14;
    result.m14 = a.m14 * b.m11 + a.m24 * b.m12 + a.m34 * b.m13 + a.m44 * b.m14;

    result.m21 = a.m11 * b.m21 + a.m21 * b.m22 + a.m31 * b.m23 + a.m41 * b.m24;
    result.m22 = a.m12 * b.m21 + a.m22 * b.m22 + a.m32 * b.m23 + a.m42 * b.m24;
    result.m23 = a.m13 * b.m21 + a.m23 * b.m22 + a.m33 * b.m23 + a.m43 * b.m24;
    result.m24 = a.m14 * b.m21 + a.m24 * b.m22 + a.m34 * b.m23 + a.m44 * b.m24;

    result.m31 = a.m11 * b.m31 + a.m21 * b.m32 + a.m31 * b.m33 + a.m41 * b.m34;
    result.m32 = a.m12 * b.m31 + a.m22 * b.m32 + a.m32 * b.m33 + a.m42 * b.m34;
    result.m33 = a.m13 * b.m31 + a.m23 * b.m32 + a.m33 * b.m33 + a.m43 * b.m34;
    result.m34 = a.m14 * b.m31 + a.m24 * b.m32 + a.m34 * b.m33 + a.m44 * b.m34;

    result.m41 = a.m11 * b.m41 + a.m21 * b.m42 + a.m31 * b.m43 + a.m41 * b.m44;
    result.m42 = a.m12 * b.m41 + a.m22 * b.m42 + a.m32 * b.m43 + a.m42 * b.m44;
    result.m43 = a.m13 * b.m41 + a.m23 * b.m42 + a.m33 * b.m43 + a.m43 * b.m44;
    result.m44 = a.m14 * b.m41 + a.m24 * b.m42 + a.m34 * b.m43 + a.m44 * b.m44;

    return result;
}

NXAffineTransform NXTransform3DGetAffineTransform(NXTransform3D t) {
    return NXAffineTransform(
        t.m11, t.m12,
        t.m21, t.m22,
        t.m41, t.m42);
}

}
