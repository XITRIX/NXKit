//
//  main2..cpp
//  NXKit
//
//  Created by Даниил Виноградов on 06.07.2022.
//

#include "Application.hpp"
#include "UIBlurView.hpp"
#include "UILabel.hpp"
#include "Label.hpp"
#include "UIStackView.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

class RootView: public UIView {
public:
    RootView() {
        UIStackView *root = new UIStackView(Axis::VERTICAL);
        root->backgroundColor = UIColor(235, 235, 235);
        stackView = root;
    //    root->backgroundColor = RED;
    //    root->cornerRadius = 40;

        UIStackView *header = new UIStackView(Axis::HORIZONTAL);
//        header->backgroundColor = UIColor(255, 0, 0);
        header->setSize(Size(AUTO, 88));
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
        footer->setSize(Size(AUTO, 73));
        footer->setBorderTop(1);
        footer->setPaddingLeft(35);
        footer->setPaddingRight(35);
        footer->setMarginLeft(30);
        footer->setMarginRight(30);

        root->addSubview(header);
        root->addSubview(body);
        root->addSubview(footer);

        addSubview(root);
    }

    void layoutSubviews() override {
        UIView::layoutSubviews();
        stackView->setSize(frame.size);
    }

private:
    UIStackView* stackView = nullptr;
};

int main(int argc, const char * argv[]) {
    Application* application = new Application();

    RootView* root = new RootView();
    application->setRootView(root);

    while (application->mainLoop());

    delete application;
    return 0;
}
