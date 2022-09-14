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

    ball = new UIControl();
    ball->setSize(Size(80, 80));
    ball->cornerRadius = 40;
    ball->transformOrigin = Point(160, 0);
    ball->backgroundColor = UIColor::red;
    ball->highlightOnFocus = false;

    view->addSubview(ball);

    setView(view);
}

void AnimationTestViewController::viewDidLoad() {
    animate(false);
}

void AnimationTestViewController::animate(bool revers) {
    UIView::animate({ ball }, 1, [this, revers]() {
        ball->transformOrigin = Point(revers ? 160 : -160, 0);
        ball->backgroundColor = revers ? UIColor::cyan : UIColor::orange;
    }, EasingFunction::exponentialInOut, [this](bool res) {
        if (!res) return;
        animate(ball->transformOrigin.x < 0);
    });
}
