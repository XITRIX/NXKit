//
//  main.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.07.2022.
//

#include "Application.hpp"
#include "UIBlurView.hpp"
#include "UILabel.hpp"
#include "UIStackView.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

int main(int argc, const char * argv[]) {
    
    Application *application = new Application();

    UIView* root = new UIView();
    root->backgroundColor = UIColor(255, 255, 255);

    application->setRootView(root);

    UIStackView* stack = new UIStackView();
    stack->backgroundColor = UIColor(255, 255, 255);

    UIView* box1 = new UIView(40, 40, 200, 200);
    box1->backgroundColor = UIColor(255, 125, 0);
    box1->cornerRadius = 24;

    UIView* box2 = new UIView(300, 20, 100, 100);
    box2->backgroundColor = UIColor(0, 125, 255);
    box2->cornerRadius = 12;

    UIView* box3 = new UIView(1270, 20, 100, 100);
    box3->backgroundColor = UIColor(0, 0, 255);
    box3->cornerRadius = 82;

    UIBlurView* blur = new UIBlurView(80, 80, 200, 200);
    blur->blurRadius = 12;
    blur->cornerRadius = 24;
    blur->borderThickness = 2;
    blur->borderColor = UIColor(155, 155, 155);
    blur->setMargins(80);
    blur->setBorderBottom(1);

    UILabel* label = new UILabel("TextTextTextTextTextTextTextText");
    label->setPosition(Point(80, 80));
    label->setSize(Size(200, 200));
//    label->frame = Rect(80, 80, 200, 200);
    label->fontSize = 24;

    UIView* stackBox1 = new UIView(0, 0, 100, 100);
    stackBox1->backgroundColor = UIColor(0, 255, 0);
    stackBox1->cornerRadius = 24;

    UIView* stackBox2 = new UIView(0, 0, 100, 100);
    stackBox2->backgroundColor = UIColor(0, 0, 255);
    stackBox2->cornerRadius = 12;

//    stack->setPosition(Point(400, 60));
    stack->addSubview(stackBox1);
    stack->addSubview(stackBox2);

    root->addSubview(box1);
    root->addSubview(box2);
    root->addSubview(box3);
    root->addSubview(blur);
    root->addSubview(label);
    root->addSubview(stack);

    bool moveRight = true;
    while (application->mainLoop()) {
        box1->setPosition(Point(box1->frame.origin.x + (moveRight ? 1 : -1), box1->frame.origin.y));
        if (box1->frame.origin.x <= 0) moveRight = true;
        if (box1->frame.origin.x >= root->frame.size.width - box1->frame.size.width) moveRight = false;
    }

    delete application;
    return 0;
}
