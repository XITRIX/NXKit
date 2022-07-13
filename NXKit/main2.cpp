//
//  main2..cpp
//  NXKit
//
//  Created by Даниил Виноградов on 06.07.2022.
//

#include "NXKit.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

class TestVC: public UIViewController {
    UIView* test;
    void loadView() override {
        UIStackView *root = new UIStackView(Axis::VERTICAL);
        root->backgroundColor = UIColor(235, 235, 235);
    //    root->backgroundColor = RED;
    //    root->cornerRadius = 40;

        UIStackView *header = new UIStackView(Axis::HORIZONTAL);
//        header->backgroundColor = UIColor(255, 0, 0);
        header->setSize(Size(UIView::AUTO, 88));
        header->setBorderBottom(1);
        header->setPaddingLeft(35);
        header->setPaddingRight(35);
        header->setMarginLeft(30);
        header->setMarginRight(30);
        header->setJustifyContent(JustifyContent::FLEX_START);
        header->setAlignItems(AlignItems::CENTER);

        UILabel *headerLabel = new UILabel("Demo app");
//        headerLabel->setSize(Size(200, 32));
        headerLabel->getFont()->fontSize = 28;
//        headerLabel->setText("Moonlight");
//        headerLabel->horizontalAlign = HorizontalAlign::CENTER;
        headerLabel->setMarginTop(7);
        headerLabel->backgroundColor = UIColor(255, 0, 0);
//
        header->addSubview(headerLabel);

        UIView *body = new UIView();
        body->backgroundColor = UIColor(0, 255, 255);
        body->setGrow(1);
        body->cornerRadius = 42;
        body->setMargins(20);

        UIStackView *footer = new UIStackView(Axis::HORIZONTAL);
//        footer->backgroundColor = UIColor(255, 255, 0);
        footer->setSize(Size(UIView::AUTO, 73));
        footer->setBorderTop(1);
        footer->setPaddingLeft(35);
        footer->setPaddingRight(35);
        footer->setMarginLeft(30);
        footer->setMarginRight(30);

        root->addSubview(header);
        root->addSubview(body);
        root->addSubview(footer);

        test = body;
        setView(root);
    }

    void viewDidLoad() override {
//        getView()->layoutSubviews();
//        test->animate(10, [this]() {
//            test->setSize(Size(10, UIView::AUTO));
//        });
    }

    void viewWillAppear(bool animated) override {
        getView()->layoutSubviews();

//        test->transformOrigin.reset();
//        test->transformOrigin.addStep({100, 100}, 1200);
//        test->transformOrigin.start();

        test->transformSize = Size(0.8f, 0.8f);
    }
};

int main(int argc, const char * argv[]) {
    Application* application = new Application();

//    RootView* root = new RootView();
    UIWindow* window = new UIWindow();
    TestVC* vc = new TestVC();
    window->setRootViewController(vc);
    window->makeKeyAndVisible();

    while (application->mainLoop());

    delete application;
    return 0;
}
