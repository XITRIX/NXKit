//
//  AnimationTestViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 28.08.2022.
//

#pragma once

#import <NXKit.hpp>

class AnimationTestViewController: public UIViewController {
public:
    AnimationTestViewController();
    void loadView() override;
    void viewDidLoad() override;
private:
    UIView* ball = nullptr;

    void animate(bool revers);
};
