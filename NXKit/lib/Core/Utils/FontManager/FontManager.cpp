//
//  FontManager.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 06.07.2022.
//

#include <Core/Utils/FontManager/FontManager.hpp>
#include <Core/Application/Application.hpp>

#include <nanovg.h>


#ifdef __SWITCH__
#include <switch.h>
#endif

namespace NXKit {

FontManager* FontManager::shared() {
    if (FontManager::_shared == nullptr) {
        FontManager::_shared = new FontManager();
    }
    return FontManager::_shared;
}

FontManager::FontManager() {
#ifdef __SWITCH__
    Result rc;
    PlFontData font;
    rc = plGetSharedFontByType(&font, PlSharedFontType_Standard);
    if (R_SUCCEEDED(rc))
        primaryFont = nvgCreateFontMem(Application::shared()->getVideoContext()->getNVGContext(), "regular", (unsigned char*)font.address, font.size, false);
    else
        exit(0);
#else
    auto path = Application::shared()->getResourcesPath() + "Fonts/switch_font.ttf";
    printf("Path: -> %s\n", path.c_str());
    primaryFont = nvgCreateFont(Application::shared()->getVideoContext()->getNVGContext(), "Switch", path.c_str());
#endif
}

}
