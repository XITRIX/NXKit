//
//  FontManager.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 06.07.2022.
//

#pragma once

class FontManager {
public:
    static FontManager* shared();

    FontManager();

    int getPrimaryFont() { return primaryFont; }
private:
    inline static FontManager* _shared = nullptr;

    int primaryFont = -1;
};
