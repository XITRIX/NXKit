//
//  UITraitCollection.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 10.09.2022.
//

#include <Core/UITraitCollection/UITraitCollection.hpp>

namespace NXKit {

UITraitCollection UITraitCollection::current;

UITraitCollection::UITraitCollection(std::vector<UITraitCollection> traitCollections) {
    for (UITraitCollection collection: traitCollections) {
        if (userInterfaceStyle == UIUserInterfaceStyle::unspecified) {
            userInterfaceStyle = collection.userInterfaceStyle;
        }
    }
}

}
