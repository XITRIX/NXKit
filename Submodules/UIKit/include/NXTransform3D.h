//
//  NXTransform3D.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 19.08.2022.
//

#pragma once

#include "Geometry.h"
#include <NXAffineTransform.h>
#include "include/core/SkM44.h"

namespace NXKit {

struct Vector3 {
    NXFloat x, y, z;
};

struct NXTransform3D {
public:
    NXFloat m11; NXFloat m12; NXFloat m13; NXFloat m14;
    NXFloat m21; NXFloat m22; NXFloat m23; NXFloat m24;
    NXFloat m31; NXFloat m32; NXFloat m33; NXFloat m34;
    NXFloat m41; NXFloat m42; NXFloat m43; NXFloat m44;

    NXTransform3D(NXFloat m11, NXFloat m12, NXFloat m13, NXFloat m14,
                  NXFloat m21, NXFloat m22, NXFloat m23, NXFloat m24,
                  NXFloat m31, NXFloat m32, NXFloat m33, NXFloat m34,
                  NXFloat m41, NXFloat m42, NXFloat m43, NXFloat m44);
    NXTransform3D();
    NXTransform3D(NXFloat* unsafePointer);

    NXTransform3D translationBy(NXFloat x, NXFloat y, NXFloat z) const;
    NXTransform3D scaleBy(NXFloat x, NXFloat y, NXFloat z) const;
    NXTransform3D scale(NXFloat factor) const;
    NXTransform3D rotationBy(NXFloat angle, NXFloat x, NXFloat y, NXFloat z) const;

    NXTransform3D concat(const NXTransform3D& other) const;
    bool operator==(const NXTransform3D& rhs) const;
    NXTransform3D operator+(const NXTransform3D& rhs) const;
    NXTransform3D operator-(const NXTransform3D& rhs) const;
    NXTransform3D operator*(const NXTransform3D& first) const;
    NXTransform3D operator*(const NXFloat& first) const;
    NXTransform3D interpolateTo(const NXTransform3D& matrix, const NXFloat& progress) const;

    Vector3 transformingVector(NXFloat x, NXFloat y, NXFloat z) const;

    static const NXTransform3D identity;

    void setAsSDLgpuMatrix() const;

    SkM44 toSkM44() const;
};

const NXTransform3D NXTransform3DIdentity = NXTransform3D(1, 0, 0, 0,
                                                          0, 1, 0, 0,
                                                          0, 0, 1, 0,
                                                          0, 0, 0, 1);

bool NXTransform3DEqualToTransform(const NXTransform3D& a, const NXTransform3D& b);

NXTransform3D NXTransform3DMakeAffineTransform(NXAffineTransform transform);
NXTransform3D CATransform3DMakeTranslation(NXFloat tx, NXFloat ty, NXFloat tz);
NXTransform3D CATransform3DMakeScale(NXFloat sx, NXFloat sy, NXFloat sz);
NXTransform3D CATransform3DMakeRotation(NXFloat angle, NXFloat x, NXFloat y, NXFloat z);
NXTransform3D CATransform3DConcat(const NXTransform3D& a, const NXTransform3D& b);
NXAffineTransform NXTransform3DGetAffineTransform(NXTransform3D t);

}
