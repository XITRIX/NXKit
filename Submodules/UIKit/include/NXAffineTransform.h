//
//  NXAffineTransform.h
//  NXKit
//
//  Created by Даниил Виноградов on 19.07.2022.
//

#pragma once

#include "Geometry.h"
#include <optional>

namespace NXKit {

struct NXAffineTransform {
public:
    float m11, m12, m21, m22, tX, tY;

    NXAffineTransform();
    NXAffineTransform(NXFloat m11, NXFloat m12, NXFloat m21, NXFloat m22, NXFloat tX, NXFloat tY);

    static NXAffineTransform translationBy(NXFloat x, NXFloat y);
    static NXAffineTransform scaleBy(NXFloat x, NXFloat y);
    static NXAffineTransform scale(NXFloat factor);
    static NXAffineTransform rotationBy(NXFloat angle);

    [[nodiscard]] std::optional<NXAffineTransform> inverted() const;
    [[nodiscard]] bool isIdentity() const;

    static const NXAffineTransform identity;

    bool operator==(const NXAffineTransform& rhs) const;
    NXAffineTransform operator*(const NXAffineTransform& rhs) const;
};

}