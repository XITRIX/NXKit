//
//  NXAffineTransform.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 19.07.2022.
//

#include <Core/NXAffineTransform/NXAffineTransform.hpp>
#include <Core/NXTransform3D/NXTransform3D.hpp>
#include <Core/Utils/Tools/Tools.hpp>

namespace NXKit {

const NXAffineTransform NXAffineTransform::identity = NXAffineTransform(1, 0, 0, 1, 0, 0);

NXAffineTransform::NXAffineTransform() {}

NXAffineTransform::NXAffineTransform(float m11, float m12, float m21, float m22, float tX, float tY):
m11(m11), m12(m12), m21(m21), m22(m22), tX(tX), tY(tY)
{ }

NXAffineTransform NXAffineTransform::translationBy(float x, float y) {
    return NXAffineTransform(
        1,   0,
        0,   1,
        x,   y);
}

NXAffineTransform NXAffineTransform::scaleBy(float x, float y) {
    return NXAffineTransform(
        x,   0,
        0,   y,
        0,   0);
}

NXAffineTransform NXAffineTransform::scale(float factor) {
    return NXAffineTransform(
        factor,         0,
             0,    factor,
             0,         0);
}

std::optional<NXAffineTransform> NXAffineTransform::inverted() const {
    auto d = m11 * m22 - m12 * m21;
    if (d < 0) return std::nullopt;

    NXAffineTransform transform;
    float multiplyer = 1 / d;

    transform.m11 = m22 * multiplyer;
    transform.m12 = -m12 * multiplyer;
    transform.m21 = -m21 * multiplyer;
    transform.m22 = m11 * multiplyer;

    return transform;
}

void NXAffineTransform::apply(NXAffineTransform t) {
    IFNNULL(this->m11, t.m11);
    IFNNULL(this->m12, t.m12);
    IFNNULL(this->m21, t.m21);
    IFNNULL(this->m22, t.m22);
    IFNNULL(this->tX, t.tX);
    IFNNULL(this->tY, t.tY);
}

NXAffineTransform NXAffineTransform::concat(const NXAffineTransform& other) const {
    return NXAffineTransformConcat(*this, other);
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

void NXAffineTransform::fillAnimationContext(std::deque<float>* context) {
    context->push_back(m11);
    context->push_back(m12);
    context->push_back(m21);
    context->push_back(m22);
    context->push_back(tX);
    context->push_back(tY);
}

void NXAffineTransform::apply(std::deque<float>* context) {
    IFNNULL(m11, pop(context));
    IFNNULL(m12, pop(context));
    IFNNULL(m21, pop(context));
    IFNNULL(m22, pop(context));
    IFNNULL(tX, pop(context));
    IFNNULL(tY, pop(context));
}

NXAffineTransform NXAffineTransform::fromAnimationContext(std::deque<float>* context) {
    auto m11 = pop(context);
    auto m12 = pop(context);
    auto m21 = pop(context);
    auto m22 = pop(context);
    auto tX = pop(context);
    auto tY = pop(context);

    return NXAffineTransform(m11, m12, m21, m22, tX, tY);
}

NXAffineTransform NXAffineTransformConcat(const NXAffineTransform& a, const NXAffineTransform& b) {
    auto ta = NXTransform3DMakeAffineTransform(a);
    auto tb = NXTransform3DMakeAffineTransform(b);

    return NXTransform3DGetAffineTransform(ta.concat(tb));
}

}
