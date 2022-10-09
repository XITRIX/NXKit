//
//  ViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#pragma once

#include "NXKit.hpp"

using namespace NXKit;

class MyViewController: public UIViewController {
public:
    MyViewController();
    
    void loadView() override;
    void viewDidLoad() override;
    void viewDidLayoutSubviews() override;
    
    void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> withEvent) override;
    void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> withEvent) override;
    void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> withEvent) override;
    
private:
    std::shared_ptr<UIScrollView> scroll;
    std::shared_ptr<UILabel> text;
};
