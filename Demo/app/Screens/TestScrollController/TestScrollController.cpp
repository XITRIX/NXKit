#include <Screens/TestScrollController/TestScrollController.hpp>
#include "romfs/romfs.hpp"

using namespace NXKit;

TestScrollController::TestScrollController() {
    setTitle("Yoga test VC");
}

void TestScrollController::loadView() {
    auto rootView = new_shared<UIView>();
    rootView->setBackgroundColor(UIColor::secondarySystemBackground);

    scrollView = new_shared<UIScrollView>();
    rootView->addSubview(scrollView);

    label = new_shared<UILabel>();
    label->setText("Just some text\nJust some text\nJust some text\nJust some text\nJust some text\nJust some text\nJust some text\nJust some textJust some textJust some textJust some textJust some textJust some textJust some textJust some textJust some textJust some textJust some textJust some textJust some text\nJust some text\nJust some text\n");

    scrollView->addSubview(label);
    scrollView->setBackgroundColor(UIColor::systemRed);
    scrollView->setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::always);
    scrollView->setBounceHorizontally(false);
    scrollView->setBounceVertically(true);

    scrollView->configureLayout([](std::shared_ptr<YGLayout> layout) {
        layout->setWidth(100_percent);
        layout->setHeight(100_percent);
    });
    rootView->configureLayout([](std::shared_ptr<YGLayout> layout) {
        layout->setAlignItems(YGAlignStretch);
    });

    setView(rootView);
}



void TestScrollController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    // label->sizeToFit();
    auto safeArea = scrollView->safeAreaInsets();
    auto availableWidth = fmaxf(0, scrollView->bounds().width() - safeArea.left - safeArea.right);
    auto fittingSize = label->sizeThatFits({ availableWidth, 100000 });
    label->setFrame({ 0, 0, availableWidth, fittingSize.height });
}
