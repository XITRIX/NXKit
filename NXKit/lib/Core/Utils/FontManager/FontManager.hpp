//
//  FontManager.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 06.07.2022.
//

#pragma once

namespace NXKit {

class FontManager {
public:
    static FontManager* shared();

    FontManager();

    int getPrimaryFont() { return primaryFont; }
    int getIconsFont() { return iconsFont; }
private:
    inline static FontManager* _shared = nullptr;

    void loadRegularFont();
    void loadIconsFont();

    int primaryFont = -1;
    int iconsFont = -1;
};

}
