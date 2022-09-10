//
//  UITraitCollection.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 10.09.2022.
//

#pragma once

#include <vector>

namespace NXKit {

enum class UIUserInterfaceStyle {
    unspecified = 0,
    light = 1,
    dark = 2
};

class UITraitCollection {
public:
    static UITraitCollection current;

    UITraitCollection() {}
    UITraitCollection(std::vector<UITraitCollection> traitCollections);
//    UIUserInterfaceStyle getUserInterfaceStyle();

    UIUserInterfaceStyle userInterfaceStyle = UIUserInterfaceStyle::unspecified;
    
};

class UITraitEnvironment {
public:
    virtual UITraitCollection getTraitCollection() = 0;
    virtual void traitCollectionDidChange(UITraitCollection previousTraitCollection) {}
};

}
