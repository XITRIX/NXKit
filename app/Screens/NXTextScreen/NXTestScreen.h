//
// Created by Даниил Виноградов on 22.02.2025.
//

#pragma once
#include <UIKit.h>

using namespace NXKit;

class NXNavigationController: public UIViewController {
public:
    void loadView() override;
    void viewDidLoad() override;
};

class NXTestScreen: public UIViewController {
public:
    void loadView() override;
    void viewDidLoad() override;
};
