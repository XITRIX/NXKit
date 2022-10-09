//
//  UIAppDelegate.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 17.07.2022.
//

#pragma once

namespace NXKit {

class UIAppDelegate: public enable_shared_from_base<UIAppDelegate> {
public:
    virtual ~UIAppDelegate() {}
    void applicationDidFinishLaunching();
};

}
