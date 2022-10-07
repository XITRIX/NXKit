//
//  TextTestController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 07.10.2022.
//

#pragma once

#include <NXKit.hpp>

using namespace NXKit;

class TextTestController: public UIViewController {
public:
    void loadView() override;
    void viewDidLoad() override;

private:
    UILabel* label;
};
