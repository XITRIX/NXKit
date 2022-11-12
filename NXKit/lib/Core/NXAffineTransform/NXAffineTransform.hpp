//
//  NXAffineTransform.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 19.07.2022.
//

#pragma once

#include <optional>
#include <deque>

namespace NXKit {

struct NXAffineTransform {
public:
    float m11, m12, m21, m22, tX, tY;

    NXAffineTransform();
    NXAffineTransform(float m11, float m12, float m21, float m22, float tX, float tY);

    static NXAffineTransform translationBy(float x, float y);
    static NXAffineTransform scaleBy(float x, float y);
    static NXAffineTransform scale(float factor);

    void apply(NXAffineTransform t);

    NXAffineTransform concat(const NXAffineTransform& other) const;
    std::optional<NXAffineTransform> inverted() const;
    bool isIdentity() const;

    static const NXAffineTransform identity;

    bool operator==(const NXAffineTransform& rhs) const;

    void apply(std::deque<float>* context);
    void fillAnimationContext(std::deque<float>* context);
    static NXAffineTransform fromAnimationContext(std::deque<float>* context);
};

NXAffineTransform NXAffineTransformConcat(const NXAffineTransform& a, const NXAffineTransform& b);

}
