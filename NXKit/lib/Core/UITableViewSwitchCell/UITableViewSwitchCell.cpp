//
//  UITableViewSwitchCell.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 13.09.2022.
//

#include <Core/UITableViewSwitchCell/UITableViewSwitchCell.hpp>

namespace NXKit {

UITableViewSwitchCell::UITableViewSwitchCell() {
    addAction(BUTTON_A, UIAction([this]() {
        setOn(!state);
    }, "OK", true, true, []() { return true; }));

    detailLabel->clipToBounds = false;
}

void UITableViewSwitchCell::setOn(bool on, bool animated) {
    this->state = on;

    if (animated) {
        detailLabel->animate(0.06, [this]() {
            detailLabel->transform = NXAffineTransform::translationBy(detailLabel->getBounds().width() * 0.1f, 0).concat(NXAffineTransform::scale(0.8f));
//            detailLabel->getFont()->fontSize = 17.6f;
        }, EasingFunction::quarticOut, [this](bool res) {
            if (!res) return;

            updateUI();
            detailLabel->animate(0.06, [this]() {
                detailLabel->transform = NXAffineTransform::identity;
//                detailLabel->getFont()->fontSize = 22;
            }, EasingFunction::quarticIn, [this](bool res) {
//                setNeedsLayout();
            });
        });
    }
    else
    {
        updateUI();
    }
}

void UITableViewSwitchCell::updateUI() {
    detailLabel->setText(state ? "ВКЛ." : "ВЫКЛ.");
    detailLabel->textColor = state ? getTintColor() : UIColor::gray;
}

void UITableViewSwitchCell::tintColorDidChange() {
    UIView::tintColorDidChange();
    detailLabel->textColor = state ? getTintColor() : UIColor::gray;
}

}
