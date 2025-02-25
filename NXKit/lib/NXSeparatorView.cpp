//
// Created by Даниил Виноградов on 24.02.2025.
//

#include <NXSeparatorView.h>

using namespace NXKit::yoga::literals;
using namespace NXKit;

NXSeparatorView::NXSeparatorView() {
    setAutolayoutEnabled(true);
    setBackgroundColor(UIColor::separator);

    configureLayout([](const std::shared_ptr <YGLayout> &layout) {
        layout->setHeight(1_pt);
        layout->setMarginVertical(14.5_pt);
    });
}