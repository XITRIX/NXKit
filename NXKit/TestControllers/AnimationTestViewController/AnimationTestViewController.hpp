//
//  AnimationTestViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 28.08.2022.
//

#pragma once

#import <NXKit.hpp>

using namespace NXKit;

class AnimationTestViewController: public UIViewController {
public:
    AnimationTestViewController();
    void loadView() override;
    void viewDidLoad() override;
private:
    std::shared_ptr<UIControl> ball;

    void animate(bool revers);

    std::vector<UIColor> colors;
};
