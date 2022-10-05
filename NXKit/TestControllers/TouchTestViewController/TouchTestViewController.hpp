//
//  TouchTestViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 13.08.2022.
//

#pragma once

#include <NXKit.hpp>

using namespace NXKit;

class TargetView: public UIControl {
    void layoutSubviews() override;
};

class TouchTestViewController: public UIViewController {
public:
    TouchTestViewController();
    
    void loadView() override;
    void viewDidLoad() override;
    void viewWillLayoutSubviews() override;
};
