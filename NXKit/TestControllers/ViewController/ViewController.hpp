//
//  ViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#pragma once

#include "NXKit.hpp"

class MyViewController: public UIViewController {
public:
    void loadView() override;
    void viewDidLoad() override;
    void viewDidLayoutSubviews() override;
    
    void touchesBegan(std::vector<UITouch*> touches, UIEvent* withEvent) override;
    void touchesMoved(std::vector<UITouch*> touches, UIEvent* withEvent) override;
    void touchesEnded(std::vector<UITouch*> touches, UIEvent* withEvent) override;
    
private:
    UIScrollView* scroll = nullptr;
    UILabel* text = nullptr;
};
