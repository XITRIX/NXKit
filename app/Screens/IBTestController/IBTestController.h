//
// Created by Daniil Vinogradov on 28/01/2025.
//

#pragma once
#include <UIKit.h>

using namespace NXKit;

class IBTestController: public UIViewController {
public:
    void loadView() override;
    void viewDidLoad() override;

private:
    IBOutlet(UILabel, text)
};
