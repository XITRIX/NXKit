//
//  NXAffineTransform.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 19.07.2022.
//

#include "NXAffineTransform.hpp"

namespace NXKit {

NXAffineTransform::NXAffineTransform() {}

NXAffineTransform::NXAffineTransform(float m11, float m12, float m21, float m22, float tX, float tY):
m11(m11), m12(m12), m21(m21), m22(m22), tX(tX), tY(tY)
{ }

NXAffineTransform NXAffineTransform::translationBy(float x, float y) {
    return NXAffineTransform();
}

}
