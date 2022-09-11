//
//  SwitchVideoContext.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 21.08.2022.
//

#include <Platforms/Switch/SwitchVideoContext.hpp>

#include <switch.h>

namespace NXKit {

UIUserInterfaceStyle SwitchVideoContext::getUserInterfaceStyle() {
    ColorSetId colorSetId;
    setsysGetColorSetId(&colorSetId);

    if (colorSetId == ColorSetId_Dark)
        return UIUserInterfaceStyle::dark;
    else
        return UIUserInterfaceStyle::light;
}

}
