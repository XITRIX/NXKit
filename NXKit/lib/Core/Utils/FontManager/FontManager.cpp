//
//  FontManager.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 06.07.2022.
//

#include "FontManager.hpp"
#include "Application.hpp"

#include <nanovg.h>

FontManager* FontManager::shared() {
    if (FontManager::_shared == nullptr) {
        FontManager::_shared = new FontManager();
    }
    return FontManager::_shared;
}

FontManager::FontManager() {
    primaryFont = nvgCreateFont(Application::shared()->getVideoContext()->getNVGContext(), "Switch", "Fonts/switch_font.ttf");
}
