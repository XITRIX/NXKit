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
    auto view = NXKit::make_shared<UIStackView>();
    view->setAlignItems(AlignItems::CENTER);
    view->setJustifyContent(JustifyContent::CENTER);

    ball = NXKit::make_shared<UIControl>();
    ball->setSize(Size(80, 80));
    ball->cornerRadius = 40;
    ball->transform = NXAffineTransform::translationBy(160, 0);
    ball->backgroundColor = UIColor::red;
    ball->highlightOnFocus = false;

    view->addSubview(ball);

    setView(view);
}

void AnimationTestViewController::viewDidLoad() {
    colors = { UIColor::cyan, UIColor::orange, UIColor::green, UIColor::systemTint, UIColor::red };
    animate(false);
}

void AnimationTestViewController::animate(bool revers) {
    static int counter = 0;

    UIView::animate({ ball }, 1, [this, revers]() {
        ball->transform = NXAffineTransform::translationBy(revers ? 160 : -160, 0);
        ball->backgroundColor = colors[counter];
        ++counter %= colors.size();
    }, EasingFunction::exponentialInOut, [this](bool res) {
        if (!res) return;
        animate(ball->transform.tX < 0);
    });
}
