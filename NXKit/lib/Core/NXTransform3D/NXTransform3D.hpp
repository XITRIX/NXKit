//
//  NXTransform3D.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 19.08.2022.
//

#pragma once

#include <Core/NXAffineTransform/NXAffineTransform.hpp>

namespace NXKit {

struct Vector3 {
    float x, y, z;
};

struct NXTransform3D {
public:
    float m11; float m12; float m13; float m14;
    float m21; float m22; float m23; float m24;
    float m31; float m32; float m33; float m34;
    float m41; float m42; float m43; float m44;

    NXTransform3D(float m11, float m12, float m13, float m14,
                  float m21, float m22, float m23, float m24,
                  float m31, float m32, float m33, float m34,
                  float m41, float m42, float m43, float m44);
    NXTransform3D();
    NXTransform3D(float* unsafePointer);

    static NXTransform3D translationBy(float x, float y);
    static NXTransform3D scaleBy(float x, float y);
    static NXTransform3D scale(float factor);

    NXTransform3D concat(const NXTransform3D& other) const;
    bool operator==(const NXTransform3D& rhs) const;
    NXTransform3D operator*(const NXTransform3D& first) const;

    Vector3 transformingVector(float x, float y, float z) const;

    static const NXTransform3D identity;
};

const NXTransform3D NXTransform3DIdentity = NXTransform3D(1, 0, 0, 0,
                                                          0, 1, 0, 0,
                                                          0, 0, 1, 0,
                                                          0, 0, 0, 1);

bool NXTransform3DEqualToTransform(const NXTransform3D& a, const NXTransform3D& b);

NXTransform3D NXTransform3DMakeAffineTransform(NXAffineTransform transform);
NXTransform3D CATransform3DMakeTranslation(float tx, float ty, float tz);
NXTransform3D CATransform3DMakeScale(float sx, float sy, float sz);
NXTransform3D CATransform3DConcat(const NXTransform3D& a, const NXTransform3D& b);
NXAffineTransform NXTransform3DGetAffineTransform(NXTransform3D t);

}
