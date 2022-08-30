//
//  Tools.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 29.08.2022.
//

#include <Core/Utils/Tools/Tools.hpp>

namespace NXKit {

float ntz(float value) {
    return isnan(value) ? 0.0f : value;
}

}
