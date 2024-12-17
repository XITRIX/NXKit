//
//  NXTransform3D.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 19.08.2022.
//

#include <NXTransform3D.h>
#include <Geometry.h>

#define RAD_PER_DEG 0.017453293f

namespace NXKit {

const NXTransform3D NXTransform3D::identity = NXTransform3DIdentity;

NXTransform3D::NXTransform3D(
        NXFloat m11, NXFloat m12, NXFloat m13, NXFloat m14,
        NXFloat m21, NXFloat m22, NXFloat m23, NXFloat m24,
        NXFloat m31, NXFloat m32, NXFloat m33, NXFloat m34,
        NXFloat m41, NXFloat m42, NXFloat m43, NXFloat m44)
        :
        m11(m11), m12(m12), m13(m13), m14(m14),
        m21(m21), m22(m22), m23(m23), m24(m24),
        m31(m31), m32(m32), m33(m33), m34(m34),
        m41(m41), m42(m42), m43(m43), m44(m44)
{ }

NXTransform3D::NXTransform3D() :
        m11(0), m12(0), m13(0), m14(0),
        m21(0), m22(0), m23(0), m24(0),
        m31(0), m32(0), m33(0), m34(0),
        m41(0), m42(0), m43(0), m44(0)
{ }

NXTransform3D::NXTransform3D(NXFloat* buffer) :
        m11(buffer[0]), m12(buffer[1]), m13(buffer[2]), m14(buffer[3]),
        m21(buffer[4]), m22(buffer[5]), m23(buffer[6]), m24(buffer[7]),
        m31(buffer[8]), m32(buffer[9]), m33(buffer[10]), m34(buffer[11]),
        m41(buffer[12]), m42(buffer[13]), m43(buffer[14]), m44(buffer[15])
{ }


NXTransform3D NXTransform3D::concat(const NXTransform3D& other) const {
    return CATransform3DConcat(*this, other);
}

bool NXTransform3D::operator==(const NXTransform3D& rhs) const {
    return NXTransform3DEqualToTransform(*this, rhs);
}

NXTransform3D NXTransform3D::operator+(const NXTransform3D& b) const {
    NXTransform3D a = *this;
    return NXTransform3D(
            a.m11 + b.m11, a.m12 + b.m12, a.m13 + b.m13, a.m14 + b.m14,
            a.m21 + b.m21, a.m22 + b.m22, a.m23 + b.m23, a.m24 + b.m24,
            a.m31 + b.m31, a.m32 + b.m32, a.m33 + b.m33, a.m34 + b.m34,
            a.m41 + b.m41, a.m42 + b.m42, a.m43 + b.m43, a.m44 + b.m44
    );
}

NXTransform3D NXTransform3D::operator-(const NXTransform3D& b) const {
    NXTransform3D a = *this;
    return NXTransform3D(
            a.m11 - b.m11, a.m12 - b.m12, a.m13 - b.m13, a.m14 - b.m14,
            a.m21 - b.m21, a.m22 - b.m22, a.m23 - b.m23, a.m24 - b.m24,
            a.m31 - b.m31, a.m32 - b.m32, a.m33 - b.m33, a.m34 - b.m34,
            a.m41 - b.m41, a.m42 - b.m42, a.m43 - b.m43, a.m44 - b.m44
    );
}

NXTransform3D NXTransform3D::operator*(const NXTransform3D& first) const {
    return (*this).concat(first);
}

NXTransform3D NXTransform3D::operator*(const NXFloat& b) const {
    NXTransform3D a = *this;
    return NXTransform3D(
            a.m11 * b, a.m12 * b, a.m13 * b, a.m14 * b,
            a.m21 * b, a.m22 * b, a.m23 * b, a.m24 * b,
            a.m31 * b, a.m32 * b, a.m33 * b, a.m34 * b,
            a.m41 * b, a.m42 * b, a.m43 * b, a.m44 * b
    );
}

void getPartsFromMatrix(const NXAffineTransform& matrix, NXFloat* angle, NXPoint* translation, NXPoint* scale) {
    auto a = NXFloat(matrix.m11);
    auto b = NXFloat(matrix.m12);
    auto c = NXFloat(matrix.m21);
    auto d = NXFloat(matrix.m22);
    auto e = NXFloat(matrix.tX);
    auto f = NXFloat(matrix.tY);

    auto delta = a * d - b * c;

    *angle = 0;
    *translation = NXPoint(e, f);
    *scale = NXPoint(1, 1);

    // Apply the QR-like decomposition.
    if (a != 0 || b != 0) {
        auto r = sqrtf(a * a + b * b);
        *angle = b > 0 ? acosf(a / r) : -acosf(a / r);
        *scale = NXPoint(r, delta / r);
    } else if (c != 0 || d != 0) {
        auto s = sqrtf(c * c + d * d);
        *angle = M_PI / 2 - (d > 0 ? acosf(-c / s) : -acosf(c / s));
        *scale = NXPoint(delta / s, s);
    } else { }

    *angle = *angle / RAD_PER_DEG;
}

NXTransform3D NXTransform3D::interpolateTo(const NXTransform3D& matrix, const NXFloat& progress) const {
    auto currentM = NXTransform3DGetAffineTransform(*this);
    auto newM = NXTransform3DGetAffineTransform(matrix);

    NXFloat angle;
    NXPoint translation, scale;

    NXFloat dAngle;
    NXPoint dTranslation, dScale;

    getPartsFromMatrix(currentM, &angle, &translation, &scale);
    getPartsFromMatrix(newM, &dAngle, &dTranslation, &dScale);
//
    auto from = angle;
    angle = angle + (dAngle - angle) * progress;
    scale = scale + (dScale - scale) * progress;// (scale - NXPoint(1, 1)) * progress + NXPoint(1, 1);
    translation = translation + (dTranslation - translation) * progress;
//    printf("Angle: from - %f | to - %f | %f - %f\n", from, dAngle, angle, progress);

    auto affineResult = NXAffineTransform::translationBy(translation.x, translation.y) * NXAffineTransform::scaleBy(scale.x, scale.y) * NXAffineTransform::rotationBy(angle);
    return NXTransform3DMakeAffineTransform(affineResult);
//    return NXTransform3D::translationBy(translation.x, translation.y, 0) * NXTransform3D::scaleBy(scale.x, scale.y, 1) * NXTransform3D::rotationBy(angle, 0, 0, 1);
}

Vector3 NXTransform3D::transformingVector(NXFloat x, NXFloat y, NXFloat z) const {
    auto newX = m11 * x + m21 * y + m31 * z + m41;
    auto newY = m12 * x + m22 * y + m32 * z + m42;
    auto newZ = m13 * x + m23 * y + m33 * z + m43;
    auto newW = m14 * x + m24 * y + m34 * z + m44;

    return { newX / newW, newY / newW, newZ / newW };
}

bool NXTransform3DEqualToTransform(const NXTransform3D& a, const NXTransform3D& b) {
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

NXTransform3D CATransform3DMakeTranslation(NXFloat tx, NXFloat ty, NXFloat tz) {
    return NXTransform3D(
            1,     0,     0,    0,
            0,     1,     0,    0,
            0,     0,     1,    0,
            tx,    ty,    tz,   1);
}

NXTransform3D CATransform3DMakeScale(NXFloat tx, NXFloat ty, NXFloat tz) {
    return NXTransform3D(
            tx,     0,     0,    0,
            0,    ty,     0,    0,
            0,     0,    tz,    0,
            0,     0,     0,    1);
}

// TODO: NOT WORKING!!!!!!!
NXTransform3D CATransform3DMakeRotation(NXFloat angle, NXFloat x, NXFloat y, NXFloat z) {
    NXFloat p, radians, c, s, c_, zc_, yc_, xzc_, xyc_, yzc_, xs, ys, zs;

    p = 1/sqrtf(x*x + y*y + z*z);
    x *= p; y *= p; z *= p;
    radians = angle * RAD_PER_DEG;
    c = cosf(radians);
    s = sinf(radians);
    c_ = 1 - c;
    zc_ = z*c_;
    yc_ = y*c_;
    xzc_ = x*zc_;
    xyc_ = x*y*c_;
    yzc_ = y*zc_;
    xs = x*s;
    ys = y*s;
    zs = z*s;

    return NXTransform3D(
            x*x*c_ + c, xyc_ + zs, xzc_ - ys, 0,
            xyc_ - zs, y*yc_ + c, yzc_ + xs, 0,
            xzc_ + ys, yzc_ - xs, z*zc_ + c, 0,
            0, 0, 0, 1);
}

NXTransform3D CATransform3DConcat(const NXTransform3D& a, const NXTransform3D& b) {
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

NXTransform3D NXTransform3D::translationBy(NXFloat x, NXFloat y, NXFloat z) const {
    return concat(CATransform3DMakeTranslation(x, y, z));
}

NXTransform3D NXTransform3D::scaleBy(NXFloat x, NXFloat y, NXFloat z) const {
    return concat(CATransform3DMakeScale(x, y, z));
}

NXTransform3D NXTransform3D::scale(NXFloat factor) const {
    return concat(CATransform3DMakeScale(factor, factor, factor));
}

NXTransform3D NXTransform3D::rotationBy(NXFloat angle, NXFloat x, NXFloat y, NXFloat z) const {
    return concat(CATransform3DMakeRotation(angle, x, y, z));
}

SkM44 NXTransform3D::toSkM44() const {
   return SkM44(m11, m21, m31, m41,
                m12, m22, m32, m42,
                m13, m23, m33, m43,
                m14, m24, m34, m44);
}

}
