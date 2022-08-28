//
//  AnimationTestViewController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 28.08.2022.
//

#include "AnimationTestViewController.hpp"

AnimationTestViewController::AnimationTestViewController() {
    setTitle("Animation test");
}

void AnimationTestViewController::loadView() {
    auto view = new UIStackView();
    view->setAlignItems(AlignItems::CENTER);
    view->setJustifyContent(JustifyContent::CENTER);

    ball = new UIView();
    ball->setSize(Size(80, 80));
    ball->cornerRadius = 40;
    ball->transformOrigin = Point(120, 0);
    ball->backgroundColor = UIColor::red;

    view->addSubview(ball);

    setView(view);
}

void AnimationTestViewController::viewDidLoad() {
    animate(false);
}

void AnimationTestViewController::animate(bool revers) {
    ball->animate(1, [this, revers]() {
        ball->transformOrigin = Point(revers ? 120 : -120, 0);
    }, EasingFunction::elasticInOut, [this](bool res) {
        if (!res) return;
        animate(ball->transformOrigin.x < 0);
    });
}
