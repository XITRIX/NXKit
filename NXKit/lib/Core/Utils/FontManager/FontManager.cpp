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
    loadRegularFont();
    loadIconsFont();
    nvgAddFallbackFontId(Application::shared()->getContext(), primaryFont, iconsFont);
}

void FontManager::loadRegularFont() {
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
    primaryFont = nvgCreateFont(Application::shared()->getContext(), "regular", path.c_str());
#endif
}

void FontManager::loadIconsFont() {
#ifdef __SWITCH__
    Result rc;
    PlFontData font;
    rc = plGetSharedFontByType(&font, PlSharedFontType_NintendoExt);
    if (R_SUCCEEDED(rc))
        iconsFont = nvgCreateFontMem(Application::shared()->getVideoContext()->getNVGContext(), "icons", (unsigned char*)font.address, font.size, false);
    else
        exit(0);
#else
    auto path = Application::shared()->getResourcesPath() + "Fonts/switch_icons.ttf";
    printf("Path: -> %s\n", path.c_str());
    iconsFont = nvgCreateFont(Application::shared()->getContext(), "icons", path.c_str());
#endif
}

}
