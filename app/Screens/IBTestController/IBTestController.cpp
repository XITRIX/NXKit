//
// Created by Daniil Vinogradov on 28/01/2025.
//

#include "IBTestController.h"

void IBTestController::loadView() {
    auto nib = UINib::fromRes("Layout/TestScreen.xml");
    setView(nib->instantiate(&idStorage));
}

void IBTestController::viewDidLoad() {
//    text()->setText("BLAH BLAH BLAH BLAH");
//    text()->setFontSize(24);
//    text()->setTextColor(UIColor::systemOrange);
}