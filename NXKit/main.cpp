//
//  main.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.07.2022.
//

#include "Application.hpp"
#include "UIBlurView.hpp"
#include "UILabel.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

int main(int argc, const char * argv[]) {
    Application *application = new Application();

    UIView* root = new UIView();
    root->backgroundColor = UIColor(255, 255, 255);
    application->rootView = root;

    UIView* box1 = new UIView();
    box1->frame = Rect(40, 40, 200, 200);
    box1->backgroundColor = UIColor(255, 125, 0);
    box1->cornerRadius = 24;

    UIView* box2 = new UIView();
    box2->frame = Rect(300, 20, 100, 100);
    box2->backgroundColor = UIColor(0, 125, 255);
    box2->cornerRadius = 12;

    UIView* box3 = new UIView();
    box3->frame = Rect(1270, 20, 100, 100);
    box3->backgroundColor = UIColor(0, 0, 255);
    box3->cornerRadius = 82;

    UIBlurView* blur = new UIBlurView();
    blur->frame = Rect(80, 80, 200, 200);
    blur->blurRadius = 24;
    blur->cornerRadius = 24;
    blur->borderThickness = 2;
    blur->borderColor = UIColor(155, 155, 155);

    UILabel* label = new UILabel();
    label->frame = Rect(80, 80, 200, 200);
//    label->

    root->addSubview(box1);
    root->addSubview(box2);
    root->addSubview(box3);
    root->addSubview(blur);
    root->addSubview(label);

    bool moveRight = true;
    while (application->mainLoop()) {
        box1->frame.origin.x += moveRight ? 1 : -1;
        if (box1->frame.origin.x <= 0) moveRight = true;
        if (box1->frame.origin.x >= root->frame.size.width - box1->frame.size.width) moveRight = false;
    }

    delete application;
    return 0;
}
